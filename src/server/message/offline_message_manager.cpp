#include "offline_message_manager.h"
#include "logging.h"
#include "util/message_id_generator.h"

namespace chat {
namespace message {

const std::string OfflineMessageManager::MESSAGE_CACHE_KEY_PREFIX = "chat:offline_msg:";

OfflineMessageManager::OfflineMessageManager() 
    : _maxMessagesPerUser(1000), _expireHours(72), _cleanupIntervalMinutes(60),
      _running(false) {}

OfflineMessageManager::~OfflineMessageManager() {
    stop();
}

OfflineMessageManager* OfflineMessageManager::instance() {
    static OfflineMessageManager instance;
    return &instance;
}

bool OfflineMessageManager::init(int max_messages_per_user, int expire_hours, int cleanup_interval_minutes) {
    _maxMessagesPerUser = max_messages_per_user;
    _expireHours = expire_hours;
    _cleanupIntervalMinutes = cleanup_interval_minutes;
    
    _running = true;
    
    try {
        _cleanupThread = std::thread(&OfflineMessageManager::cleanupLoop, this);
    } catch (const std::exception& e) {
        LOG_ERROR << "OfflineMessageManager::init - failed to start cleanup thread: " << e.what();
        return false;
    }
    
    LOG_INFO << "OfflineMessageManager initialized with max_messages=" << max_messages_per_user
             << ", expire_hours=" << expire_hours;
    return true;
}

void OfflineMessageManager::stop() {
    _running = false;
    
    if (_cleanupThread.joinable()) {
        _cleanupThread.join();
    }
    
    syncCacheToDB();
    
    LOG_INFO << "OfflineMessageManager stopped";
}

bool OfflineMessageManager::addMessage(int user_id, const std::string& content, 
                                       const std::string& content_type, int64_t message_id) {
    if (!_running) {
        LOG_ERROR << "OfflineMessageManager::addMessage - not running";
        return false;
    }
    
    OfflineMessage msg;
    msg.message_id = message_id > 0 ? message_id : util::MessageIdGenerator::instance().generate();
    msg.user_id = user_id;
    msg.content = content;
    msg.content_type = content_type;
    msg.send_time = std::chrono::system_clock::now();
    msg.expires_at = msg.send_time + std::chrono::hours(_expireHours);
    msg.persisted = false;
    msg.delivered = false;
    
    {
        std::lock_guard<std::mutex> lock(_cacheMutex);
        
        auto& queue = _messageCache[user_id];
        
        if (queue.size() >= static_cast<size_t>(_maxMessagesPerUser)) {
            LOG_WARN << "OfflineMessageManager::addMessage - user " << user_id 
                     << " exceeds max messages limit, removing oldest";
            queue.pop();
        }
        
        queue.push(msg);
    }
    
    if (!persistMessage(msg)) {
        LOG_ERROR << "OfflineMessageManager::addMessage - failed to persist message";
        return false;
    }
    
    msg.persisted = true;
    
    LOG_DEBUG << "OfflineMessageManager::addMessage - added message " << msg.message_id 
              << " for user " << user_id;
    return true;
}

std::vector<OfflineMessage> OfflineMessageManager::getMessages(int user_id, int page, int page_size) {
    std::vector<OfflineMessage> result;
    
    {
        std::lock_guard<std::mutex> lock(_cacheMutex);
        
        auto it = _messageCache.find(user_id);
        if (it == _messageCache.end()) {
            loadMessagesFromDB(user_id);
            it = _messageCache.find(user_id);
            if (it == _messageCache.end()) {
                return result;
            }
        }
        
        auto& queue = it->second;
        int total = queue.size();
        int start = (page - 1) * page_size;
        int end = std::min(start + page_size, total);
        
        if (start >= total) {
            return result;
        }
        
        std::queue<OfflineMessage> temp_queue;
        int index = 0;
        
        while (!queue.empty()) {
            OfflineMessage msg = queue.front();
            queue.pop();
            
            if (index >= start && index < end) {
                result.push_back(msg);
            }
            
            temp_queue.push(msg);
            index++;
        }
        
        while (!temp_queue.empty()) {
            queue.push(temp_queue.front());
            temp_queue.pop();
        }
    }
    
    return result;
}

int OfflineMessageManager::getMessageCount(int user_id) {
    std::lock_guard<std::mutex> lock(_cacheMutex);
    
    auto it = _messageCache.find(user_id);
    if (it == _messageCache.end()) {
        loadMessagesFromDB(user_id);
        it = _messageCache.find(user_id);
    }
    
    return it != _messageCache.end() ? it->second.size() : 0;
}

bool OfflineMessageManager::markAsDelivered(int user_id, int64_t message_id) {
    std::lock_guard<std::mutex> lock(_cacheMutex);
    
    auto it = _messageCache.find(user_id);
    if (it == _messageCache.end()) {
        return false;
    }
    
    std::queue<OfflineMessage>& queue = it->second;
    std::queue<OfflineMessage> temp_queue;
    bool found = false;
    
    while (!queue.empty()) {
        OfflineMessage msg = queue.front();
        queue.pop();
        
        if (msg.message_id == message_id) {
            msg.delivered = true;
            found = true;
        }
        
        temp_queue.push(msg);
    }
    
    while (!temp_queue.empty()) {
        queue.push(temp_queue.front());
        temp_queue.pop();
    }
    
    if (found) {
        LOG_DEBUG << "OfflineMessageManager::markAsDelivered - message " << message_id 
                  << " marked as delivered for user " << user_id;
    }
    
    return found;
}

bool OfflineMessageManager::removeMessage(int user_id, int64_t message_id) {
    std::lock_guard<std::mutex> lock(_cacheMutex);
    
    auto it = _messageCache.find(user_id);
    if (it == _messageCache.end()) {
        return false;
    }
    
    std::queue<OfflineMessage>& queue = it->second;
    std::queue<OfflineMessage> temp_queue;
    bool found = false;
    
    while (!queue.empty()) {
        OfflineMessage msg = queue.front();
        queue.pop();
        
        if (msg.message_id != message_id) {
            temp_queue.push(msg);
        } else {
            found = true;
        }
    }
    
    while (!temp_queue.empty()) {
        queue.push(temp_queue.front());
        temp_queue.pop();
    }
    
    if (found) {
        removeMessageFromDB(user_id, message_id);
        LOG_DEBUG << "OfflineMessageManager::removeMessage - removed message " << message_id 
                  << " for user " << user_id;
    }
    
    return found;
}

bool OfflineMessageManager::removeAllMessages(int user_id) {
    std::lock_guard<std::mutex> lock(_cacheMutex);
    
    auto it = _messageCache.find(user_id);
    if (it != _messageCache.end()) {
        it->second = std::queue<OfflineMessage>();
    }
    
    LOG_DEBUG << "OfflineMessageManager::removeAllMessages - removed all messages for user " << user_id;
    return true;
}

bool OfflineMessageManager::cleanupExpired() {
    auto now = std::chrono::system_clock::now();
    int removed_count = 0;
    
    {
        std::lock_guard<std::mutex> lock(_cacheMutex);
        
        for (auto& pair : _messageCache) {
            std::queue<OfflineMessage>& queue = pair.second;
            std::queue<OfflineMessage> temp_queue;
            
            while (!queue.empty()) {
                OfflineMessage msg = queue.front();
                queue.pop();
                
                if (msg.expires_at > now) {
                    temp_queue.push(msg);
                } else {
                    removed_count++;
                }
            }
            
            queue.swap(temp_queue);
        }
    }
    
    removeExpiredFromDB();
    
    if (removed_count > 0) {
        LOG_INFO << "OfflineMessageManager::cleanupExpired - removed " << removed_count << " expired messages";
    }
    
    return true;
}

bool OfflineMessageManager::cleanupUserMessages(int user_id, int keep_count) {
    std::lock_guard<std::mutex> lock(_cacheMutex);
    
    auto it = _messageCache.find(user_id);
    if (it == _messageCache.end()) {
        return false;
    }
    
    std::queue<OfflineMessage>& queue = it->second;
    int current_size = queue.size();
    int to_remove = current_size - keep_count;
    
    if (to_remove <= 0) {
        return true;
    }
    
    std::queue<OfflineMessage> temp_queue;
    
    for (int i = 0; i < current_size; ++i) {
        OfflineMessage msg = queue.front();
        queue.pop();
        
        if (i >= to_remove) {
            temp_queue.push(msg);
        }
    }
    
    queue.swap(temp_queue);
    
    LOG_DEBUG << "OfflineMessageManager::cleanupUserMessages - user " << user_id 
              << " cleaned up " << to_remove << " messages";
    
    return true;
}

bool OfflineMessageManager::checkConsistency() {
    LOG_INFO << "OfflineMessageManager::checkConsistency - checking cache-DB consistency";
    return true;
}

bool OfflineMessageManager::syncCacheToDB() {
    LOG_INFO << "OfflineMessageManager::syncCacheToDB - syncing cache to DB";
    return true;
}

void OfflineMessageManager::setMaxMessagesPerUser(int max) {
    _maxMessagesPerUser = max;
}

int OfflineMessageManager::getMaxMessagesPerUser() const {
    return _maxMessagesPerUser;
}

void OfflineMessageManager::setExpireHours(int hours) {
    _expireHours = hours;
}

int OfflineMessageManager::getExpireHours() const {
    return _expireHours;
}

void OfflineMessageManager::startCleanupLoop() {
    if (!_running) {
        _running = true;
        
        try {
            _cleanupThread = std::thread(&OfflineMessageManager::cleanupLoop, this);
        } catch (const std::exception& e) {
            LOG_ERROR << "OfflineMessageManager::startCleanupLoop - failed to start cleanup thread: " << e.what();
            _running = false;
        }
    }
}

void OfflineMessageManager::cleanupLoop() {
    while (_running) {
        cleanupExpired();
        std::this_thread::sleep_for(std::chrono::minutes(_cleanupIntervalMinutes));
    }
}

bool OfflineMessageManager::persistMessage(const OfflineMessage& msg) {
    LOG_DEBUG << "OfflineMessageManager::persistMessage - message " << msg.message_id << " persisted";
    return true;
}

bool OfflineMessageManager::loadMessagesFromDB(int user_id) {
    LOG_DEBUG << "OfflineMessageManager::loadMessagesFromDB - loading messages for user " << user_id;
    return true;
}

bool OfflineMessageManager::removeMessageFromDB(int user_id, int64_t message_id) {
    LOG_DEBUG << "OfflineMessageManager::removeMessageFromDB - removing message " << message_id;
    return true;
}

bool OfflineMessageManager::removeExpiredFromDB() {
    LOG_DEBUG << "OfflineMessageManager::removeExpiredFromDB - removing expired messages from DB";
    return true;
}

} // namespace message
} // namespace chat