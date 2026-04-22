#ifndef BCRYPT_H
#define BCRYPT_H

#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <atomic>
#include <functional>

namespace chat {
namespace util {

// 盐值管理器 - 处理盐值的生成、存储和轮换
class SaltManager {
public:
    using SaltStoreCallback = std::function<bool(const std::string& key, const std::string& salt, std::time_t expires_at)>;
    using SaltLoadCallback = std::function<bool(const std::string& key, std::string& salt, std::time_t& expires_at)>;
    
    SaltManager();
    
    // 设置盐值存储回调
    void setSaltStoreCallback(SaltStoreCallback callback);
    void setSaltLoadCallback(SaltLoadCallback callback);
    
    // 获取用户盐值（不存在则生成）
    std::string getSalt(const std::string& user_id);
    
    // 轮换用户盐值
    std::string rotateSalt(const std::string& user_id);
    
    // 检查盐值是否过期
    bool isSaltExpired(const std::string& user_id) const;
    
    // 设置盐值过期时间（小时）
    void setSaltExpiryHours(int hours);
    
    // 获取盐值过期时间
    int getSaltExpiryHours() const;
    
    // 清理过期盐值缓存
    void cleanupExpiredSalts();
    
private:
    struct SaltInfo {
        std::string salt;
        std::time_t expires_at;
        bool loaded_from_storage;
    };
    
    std::unordered_map<std::string, SaltInfo> salt_cache_;
    mutable std::mutex mutex_;
    
    SaltStoreCallback store_callback_;
    SaltLoadCallback load_callback_;
    
    int salt_expiry_hours_;
    
    // 生成随机盐值
    std::string generateSalt() const;
};

// bcrypt加密器 - 支持盐值管理和成本动态调整
class BCrypt {
public:
    BCrypt();
    
    // 设置盐值管理器
    void setSaltManager(std::shared_ptr<SaltManager> manager);
    
    // 使用默认配置哈希密码
    std::string hashPassword(const std::string& password);
    
    // 使用指定盐值和成本哈希密码
    std::string hashPasswordWithSalt(const std::string& password, const std::string& salt, int cost);
    
    // 验证密码
    bool verifyPassword(const std::string& password, const std::string& hash);
    
    // 验证密码并检查是否需要重新哈希（成本升级）
    bool verifyAndCheckRehash(const std::string& password, const std::string& hash, bool& needs_rehash);
    
    // 检查哈希是否有效
    static bool isValidHash(const std::string& hash);
    
    // 获取哈希中的成本因子
    static int getCostFromHash(const std::string& hash);
    
    // 设置目标加密成本
    void setTargetCost(int cost);
    
    // 获取目标加密成本
    int getTargetCost() const;
    
    // 设置最小加密成本（低于此值需要重新哈希）
    void setMinCost(int cost);
    
    // 获取最小加密成本
    int getMinCost() const;
    
    // 根据负载动态调整加密成本
    void adjustCostBasedOnLoad(double load_factor);
    
    // 获取当前有效加密成本
    int getEffectiveCost() const;
    
private:
    static constexpr int DEFAULT_COST = 12;
    static constexpr int MIN_COST = 4;
    static constexpr int MAX_COST = 31;
    
    std::shared_ptr<SaltManager> salt_manager_;
    std::atomic<int> target_cost_;
    std::atomic<int> min_cost_;
    std::atomic<int> effective_cost_;
    
    // 内部哈希实现
    static std::string internalHash(const std::string& password, const std::string& salt, int cost);
    static bool internalVerify(const std::string& password, const std::string& hash);
    
    // Base64编码（bcrypt专用格式）
    static std::string base64Encode(const unsigned char* data, size_t length);
    static bool base64Decode(const std::string& encoded, unsigned char* data, size_t& length);
    
    // Blowfish加密核心
    static void initState(unsigned char* state, const unsigned char* key, size_t key_len);
    static void encryptBlock(unsigned char* block, unsigned char* state);
};

} // namespace util
} // namespace chat

#endif // BCRYPT_H