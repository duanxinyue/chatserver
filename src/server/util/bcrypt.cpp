#include "bcrypt.h"
#include "logging.h"
#include <random>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace chat {
namespace util {

// SaltManager 实现
SaltManager::SaltManager() : salt_expiry_hours_(24 * 30) {} // 默认30天过期

void SaltManager::setSaltStoreCallback(SaltStoreCallback callback) {
    store_callback_ = std::move(callback);
}

void SaltManager::setSaltLoadCallback(SaltLoadCallback callback) {
    load_callback_ = std::move(callback);
}

std::string SaltManager::getSalt(const std::string& user_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = salt_cache_.find(user_id);
    if (it != salt_cache_.end()) {
        // 检查是否过期
        if (std::time(nullptr) < it->second.expires_at) {
            return it->second.salt;
        }
        // 过期了，移除缓存
        salt_cache_.erase(it);
    }
    
    // 尝试从存储加载
    std::string salt;
    std::time_t expires_at;
    if (load_callback_ && load_callback_(user_id, salt, expires_at)) {
        // 检查加载的盐值是否过期
        if (std::time(nullptr) < expires_at) {
            salt_cache_[user_id] = {salt, expires_at, true};
            return salt;
        }
    }
    
    // 生成新盐值
    salt = generateSalt();
    expires_at = std::time(nullptr) + salt_expiry_hours_ * 3600;
    
    // 存储盐值
    if (store_callback_) {
        store_callback_(user_id, salt, expires_at);
    }
    
    salt_cache_[user_id] = {salt, expires_at, true};
    
    LOG_DEBUG << "SaltManager::getSalt - generated new salt for user: " << user_id;
    
    return salt;
}

std::string SaltManager::rotateSalt(const std::string& user_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 生成新盐值
    std::string new_salt = generateSalt();
    std::time_t expires_at = std::time(nullptr) + salt_expiry_hours_ * 3600;
    
    // 存储新盐值
    if (store_callback_) {
        store_callback_(user_id, new_salt, expires_at);
    }
    
    // 更新缓存
    salt_cache_[user_id] = {new_salt, expires_at, true};
    
    LOG_INFO << "SaltManager::rotateSalt - rotated salt for user: " << user_id;
    
    return new_salt;
}

bool SaltManager::isSaltExpired(const std::string& user_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = salt_cache_.find(user_id);
    if (it == salt_cache_.end()) {
        return true; // 没有缓存视为过期
    }
    
    return std::time(nullptr) >= it->second.expires_at;
}

void SaltManager::setSaltExpiryHours(int hours) {
    std::lock_guard<std::mutex> lock(mutex_);
    salt_expiry_hours_ = hours;
}

int SaltManager::getSaltExpiryHours() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return salt_expiry_hours_;
}

void SaltManager::cleanupExpiredSalts() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::time(nullptr);
    auto it = salt_cache_.begin();
    
    while (it != salt_cache_.end()) {
        if (now >= it->second.expires_at) {
            it = salt_cache_.erase(it);
        } else {
            ++it;
        }
    }
}

std::string SaltManager::generateSalt() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 255);
    
    unsigned char salt_bytes[16];
    for (int i = 0; i < 16; ++i) {
        salt_bytes[i] = static_cast<unsigned char>(dis(gen));
    }
    
    // bcrypt格式: $2a$cost$salt
    std::stringstream ss;
    ss << "$2a$12$";
    
    // 自定义base64编码（bcrypt专用）
    static const char* base64_chars = "./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    
    for (size_t i = 0; i < 16; i += 3) {
        unsigned char b0 = salt_bytes[i];
        unsigned char b1 = (i + 1 < 16) ? salt_bytes[i + 1] : 0;
        unsigned char b2 = (i + 2 < 16) ? salt_bytes[i + 2] : 0;
        
        ss << base64_chars[b0 >> 2];
        ss << base64_chars[((b0 & 0x03) << 4) | (b1 >> 4)];
        ss << base64_chars[((b1 & 0x0F) << 2) | (b2 >> 6)];
        ss << base64_chars[b2 & 0x3F];
    }
    
    return ss.str();
}

// BCrypt 实现
BCrypt::BCrypt() 
    : target_cost_(DEFAULT_COST), 
      min_cost_(MIN_COST),
      effective_cost_(DEFAULT_COST) {}

void BCrypt::setSaltManager(std::shared_ptr<SaltManager> manager) {
    salt_manager_ = std::move(manager);
}

std::string BCrypt::hashPassword(const std::string& password) {
    // 如果有盐值管理器，使用它获取盐值
    std::string salt;
    if (salt_manager_) {
        // 这里需要用户ID，暂时生成随机盐值
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(1, 1000000);
        salt = salt_manager_->getSalt(std::to_string(dis(gen)));
    } else {
        // 生成临时盐值
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 255);
        
        unsigned char salt_bytes[16];
        for (int i = 0; i < 16; ++i) {
            salt_bytes[i] = static_cast<unsigned char>(dis(gen));
        }
        
        std::stringstream ss;
        ss << "$2a$" << std::setw(2) << std::setfill('0') << effective_cost_.load() << "$";
        static const char* base64_chars = "./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        
        for (size_t i = 0; i < 16; i += 3) {
            unsigned char b0 = salt_bytes[i];
            unsigned char b1 = (i + 1 < 16) ? salt_bytes[i + 1] : 0;
            unsigned char b2 = (i + 2 < 16) ? salt_bytes[i + 2] : 0;
            
            ss << base64_chars[b0 >> 2];
            ss << base64_chars[((b0 & 0x03) << 4) | (b1 >> 4)];
            ss << base64_chars[((b1 & 0x0F) << 2) | (b2 >> 6)];
            ss << base64_chars[b2 & 0x3F];
        }
        
        salt = ss.str();
    }
    
    return internalHash(password, salt, effective_cost_.load());
}

std::string BCrypt::hashPasswordWithSalt(const std::string& password, const std::string& salt, int cost) {
    return internalHash(password, salt, cost);
}

bool BCrypt::verifyPassword(const std::string& password, const std::string& hash) {
    return internalVerify(password, hash);
}

bool BCrypt::verifyAndCheckRehash(const std::string& password, const std::string& hash, bool& needs_rehash) {
    needs_rehash = false;
    
    // 验证密码
    if (!internalVerify(password, hash)) {
        return false;
    }
    
    // 检查是否需要重新哈希
    int hash_cost = getCostFromHash(hash);
    if (hash_cost < min_cost_.load()) {
        needs_rehash = true;
        LOG_DEBUG << "BCrypt::verifyAndCheckRehash - hash cost " << hash_cost 
                  << " below minimum " << min_cost_.load() << ", needs rehash";
    }
    
    return true;
}

bool BCrypt::isValidHash(const std::string& hash) {
    if (hash.size() < 29) return false;
    
    // 检查格式: $2a$cost$salt+hash
    if (hash[0] != '$' || hash[1] != '2' || hash[2] != 'a' || hash[3] != '$') {
        return false;
    }
    
    // 检查成本因子
    std::string cost_str = hash.substr(4, 2);
    try {
        int cost = std::stoi(cost_str);
        if (cost < BCrypt::MIN_COST || cost > BCrypt::MAX_COST) {
            return false;
        }
    } catch (...) {
        return false;
    }
    
    // 检查盐值分隔符
    if (hash[6] != '$') {
        return false;
    }
    
    return true;
}

int BCrypt::getCostFromHash(const std::string& hash) {
    if (!BCrypt::isValidHash(hash)) {
        return BCrypt::DEFAULT_COST;
    }
    
    try {
        return std::stoi(hash.substr(4, 2));
    } catch (...) {
        return BCrypt::DEFAULT_COST;
    }
}

void BCrypt::setTargetCost(int cost) {
    cost = std::max(MIN_COST, std::min(MAX_COST, cost));
    target_cost_.store(cost);
    effective_cost_.store(cost);
}

int BCrypt::getTargetCost() const {
    return target_cost_.load();
}

void BCrypt::setMinCost(int cost) {
    cost = std::max(MIN_COST, std::min(MAX_COST, cost));
    min_cost_.store(cost);
}

int BCrypt::getMinCost() const {
    return min_cost_.load();
}

void BCrypt::adjustCostBasedOnLoad(double load_factor) {
    int target = target_cost_.load();
    
    // 根据负载调整成本
    // 负载 < 0.5: 使用目标成本
    // 负载 0.5-0.8: 降低1级
    // 负载 > 0.8: 降低2级
    int adjusted_cost = target;
    
    if (load_factor > 0.8) {
        adjusted_cost = std::max(MIN_COST, target - 2);
    } else if (load_factor > 0.5) {
        adjusted_cost = std::max(MIN_COST, target - 1);
    }
    
    effective_cost_.store(adjusted_cost);
    
    if (adjusted_cost != target) {
        LOG_WARN << "BCrypt::adjustCostBasedOnLoad - adjusted cost from " 
                 << target << " to " << adjusted_cost << " due to load: " << load_factor;
    }
}

int BCrypt::getEffectiveCost() const {
    return effective_cost_.load();
}

// 内部哈希实现（简化版）
std::string BCrypt::internalHash(const std::string& password, const std::string& salt, int cost) {
    // 提取盐值部分（去掉$2a$cost$前缀）
    std::string salt_part = salt;
    if (salt_part.size() > 7) {
        // 检查是否已经有前缀
        if (salt_part.substr(0, 4) == "$2a$") {
            // 提取盐值部分
            size_t dollar_pos = salt_part.find('$', 7);
            if (dollar_pos != std::string::npos) {
                salt_part = salt_part.substr(dollar_pos + 1);
            }
        }
    }
    
    // 确保盐值长度正确
    if (salt_part.size() > 22) {
        salt_part = salt_part.substr(0, 22);
    }
    
    // 生成哈希（简化实现，实际应该使用真正的bcrypt算法）
    std::string hash = "$2a$" + std::to_string(cost) + "$" + salt_part;
    
    // 添加模拟的哈希部分
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 63);
    static const char* base64_chars = "./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    
    std::string hash_part;
    for (int i = 0; i < 31; ++i) {
        hash_part += base64_chars[dis(gen)];
    }
    
    hash += hash_part;
    
    return hash;
}

bool BCrypt::internalVerify(const std::string& password, const std::string& hash) {
    if (!BCrypt::isValidHash(hash)) {
        return false;
    }
    
    // 提取盐值和成本
    int cost = BCrypt::getCostFromHash(hash);
    
    // 提取盐值部分
    size_t dollar_pos = hash.find('$', 7);
    if (dollar_pos == std::string::npos) {
        return false;
    }
    
    std::string salt_part = hash.substr(0, dollar_pos + 23); // 包含$2a$cost$和22字符盐值
    
    // 重新计算哈希并比较
    std::string computed_hash = internalHash(password, salt_part, cost);
    
    // 常量时间比较
    if (computed_hash.size() != hash.size()) {
        return false;
    }
    
    bool equal = true;
    for (size_t i = 0; i < hash.size(); ++i) {
        equal &= (computed_hash[i] == hash[i]);
    }
    
    return equal;
}

} // namespace util
} // namespace chat
