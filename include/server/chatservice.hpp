#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <unordered_map>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include "redis.hpp"
#include "friendmodel.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "chat.pb.h"

namespace chat {

/**
 * @brief 聊天服务类
 * 
 * 处理所有业务逻辑，包括用户登录、注册、消息发送、好友管理等
 * 采用单例模式，确保全局只有一个实例
 */
class ChatBusinessService {
public:
    /**
     * @brief 获取单例实例
     * @return ChatBusinessService* 单例指针
     */
    static ChatBusinessService* instance();
    
    /**
     * @brief 初始化Redis连接
     * @param host Redis主机地址
     * @param port Redis端口
     */
    void initRedis(const std::string& host, int port);
    
    /**
     * @brief 处理用户登录
     * @param conn_id 连接ID
     * @param msg Protobuf消息对象
     */
    void login(const std::string& conn_id, const ChatMessage& msg);
    
    /**
     * @brief 处理用户注册
     * @param conn_id 连接ID
     * @param msg Protobuf消息对象
     */
    void reg(const std::string& conn_id, const ChatMessage& msg);
    
    /**
     * @brief 处理单聊消息
     * @param conn_id 连接ID
     * @param msg Protobuf消息对象
     */
    void oneChat(const std::string& conn_id, const ChatMessage& msg);
    
    /**
     * @brief 处理添加好友
     * @param conn_id 连接ID
     * @param msg Protobuf消息对象
     */
    void addFriend(const std::string& conn_id, const ChatMessage& msg);
    
    /**
     * @brief 处理用户注销
     * @param conn_id 连接ID
     * @param msg Protobuf消息对象
     */
    void loginout(const std::string& conn_id, const ChatMessage& msg);
    
    /**
     * @brief 处理消息分发
     * @param conn_id 连接ID
     * @param msg Protobuf消息对象
     */
    void handleMessage(const std::string& conn_id, const ChatMessage& msg);
    
    /**
     * @brief 处理心跳消息
     * @param conn_id 连接ID
     * @param msg Protobuf消息对象
     */
    void handleHeartbeat(const std::string& conn_id, const ChatMessage& msg);
    
    /**
     * @brief 发送消息确认
     * @param conn_id 连接ID
     * @param msg_id 消息ID
     * @param error_code 错误码
     * @param error_msg 错误信息
     */
    void sendMessageAck(const std::string& conn_id, uint64_t msg_id, int error_code, const std::string& error_msg);
    
private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    ChatBusinessService();
    
    // 消息处理器类型定义
    using MsgHandler = std::function<void(const std::string&, const ChatMessage&)>;
    
    // 消息处理器映射表（消息类型 -> 处理函数）
    std::unordered_map<int, MsgHandler> _msgHandlerMap;
    
    // 互斥锁，保护共享数据
    mutable std::mutex _mutex;
    
    // 用户ID -> 连接ID 映射表
    std::unordered_map<int, std::string> _userConnMap;
    
    // 数据模型
    UserModel _userModel;           // 用户数据模型
    OfflineMsgModel _offlineMsgModel; // 离线消息数据模型
    FriendModel _friendModel;       // 好友数据模型
    Redis _redis;                   // Redis操作对象
};

} // namespace chat

#endif // CHATSERVICE_H
