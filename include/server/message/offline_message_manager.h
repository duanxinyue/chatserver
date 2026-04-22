#ifndef OFFLINE_MESSAGE_MANAGER_H
#define OFFLINE_MESSAGE_MANAGER_H

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <mutex>
#include <atomic>
#include <memory>
#include <string>
#include <chrono>
#include <functional>
#include <thread>

namespace chat {
namespace message {

struct OfflineMessage {
    int64_t message_id;
    int user_id;
    std::string content;
    std::string content_type;
    std::chrono::system_clock::time_point send_time;
    std::chrono::system_clock::time_point expires_at;
    bool persisted;
    bool delivered;
    
    OfflineMessage() : message_id(0), user_id(0), persisted(false), delivered(false) {}
};

class OfflineMessageManager {
public:
    static OfflineMessageManager* instance();
    
    bool init(int max_messages_per_user = 1000, int expire_hours = 72, 
              int cleanup_interval_minutes = 60);
    
    bool addMessage(int user_id, const std::string& content, 
                    const std::string& content_type = "text", int64_t message_id = 0);
    
    std::vector<OfflineMessage> getMessages(int user_id, int page = 1, int page_size = 50);
    int getMessageCount(int user_id);
    
    bool markAsDelivered(int user_id, int64_t message_id);
    bool removeMessage(int user_id, int64_t message_id);
    bool removeAllMessages(int user_id);
    
    bool cleanupExpired();
    bool cleanupUserMessages(int user_id, int keep_count = 0);
    
    bool checkConsistency();
    bool syncCacheToDB();
    
    void setMaxMessagesPerUser(int max);
    int getMaxMessagesPerUser() const;
    
    void setExpireHours(int hours);
    int getExpireHours() const;
    
    void startCleanupLoop();
    void stop();
    
private:
    OfflineMessageManager();
    ~OfflineMessageManager();
    
    void cleanupLoop();
    
    bool persistMessage(const OfflineMessage& msg);
    bool loadMessagesFromDB(int user_id);
    
    bool removeMessageFromDB(int user_id, int64_t message_id);
    bool removeExpiredFromDB();
    
    std::unordered_map<int, std::queue<OfflineMessage>> _messageCache;
    mutable std::mutex _cacheMutex;
    
    std::atomic<int> _maxMessagesPerUser;
    std::atomic<int> _expireHours;
    std::atomic<int> _cleanupIntervalMinutes;
    
    std::atomic<bool> _running;
    std::thread _cleanupThread;
    
    static const std::string MESSAGE_CACHE_KEY_PREFIX;
};

} // namespace message
} // namespace chat

#endif // OFFLINE_MESSAGE_MANAGER_H