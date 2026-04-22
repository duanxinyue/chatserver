#include "chatservice.hpp"
#include "logging.h"
#include "error_code.h"
#include "chatserver.hpp"
#include <vector>
#include <unordered_map>
#include <functional>
#include <string>

using namespace std;
using namespace chat;

extern ChatServer* g_chat_server;

/**
 * @brief 消息类型枚举定义
 */
enum MsgType {
    LOGIN_MSG = 1,
    LOGINOUT_MSG = 2,
    REG_MSG = 3,
    ONE_CHAT_MSG = 4,
    ADD_FRIEND_MSG = 5,
    HEARTBEAT_MSG = 6,
    LOGIN_MSG_ACK = 1000,
    REG_MSG_ACK = 1001,
    MSG_ACK = 1002,
    HEARTBEAT_ACK = 1003
};

/**
 * @brief 获取单例实例（懒汉模式，线程安全）
 */
ChatService* ChatService::instance() {
    static ChatService service;
    return &service;
}

/**
 * @brief 构造函数
 * 
 * 初始化消息处理器映射表，注册各种消息类型的处理函数
 */
ChatService::ChatService() {
    _msgHandlerMap.insert(std::make_pair(LOGIN_MSG, std::bind(&ChatService::login, this, std::placeholders::_1, std::placeholders::_2)));
    _msgHandlerMap.insert(std::make_pair(LOGINOUT_MSG, std::bind(&ChatService::loginout, this, std::placeholders::_1, std::placeholders::_2)));
    _msgHandlerMap.insert(std::make_pair(REG_MSG, std::bind(&ChatService::reg, this, std::placeholders::_1, std::placeholders::_2)));
    _msgHandlerMap.insert(std::make_pair(ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, std::placeholders::_1, std::placeholders::_2)));
    _msgHandlerMap.insert(std::make_pair(ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, std::placeholders::_1, std::placeholders::_2)));
    _msgHandlerMap.insert(std::make_pair(HEARTBEAT_MSG, std::bind(&ChatService::handleHeartbeat, this, std::placeholders::_1, std::placeholders::_2)));
}

/**
 * @brief 消息分发处理
 * 
 * 根据消息类型分发到对应的处理函数
 * 
 * @param conn_id 连接ID
 * @param msg Protobuf消息对象
 */
void ChatService::handleMessage(const string& conn_id, const ChatMessage& msg) {
    try {
        int msgid = static_cast<int>(msg.msg_type());
        auto it = _msgHandlerMap.find(msgid);
        if (it != _msgHandlerMap.end()) {
            it->second(conn_id, msg);
        } else {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
            // 构造错误响应
            ChatMessage response;
            response.set_version(1);
            response.set_msg_type(static_cast<MsgType>(MSG_ACK));
            response.set_seq(msg.seq());
            
            MsgAck ack;
            ack.set_version(1);
            ack.set_msg_type(static_cast<MsgType>(MSG_ACK));
            ack.set_seq(msg.seq());
            ack.set_user_id(0);
            ack.set_ack_seq(msg.seq());
            response.set_allocated_msg_ack(new MsgAck(ack));
            
            if (g_chat_server) {
                string serialized;
                response.SerializeToString(&serialized);
                g_chat_server->sendResponse(conn_id, serialized);
            }
        }
    } catch (const exception& e) {
        LOG_ERROR << "ChatService::handleMessage exception: " << e.what();
    }
}

/**
 * @brief 发送消息确认
 * 
 * @param conn_id 连接ID
 * @param msg_id 消息ID
 * @param error_code 错误码
 * @param error_msg 错误信息
 */
void ChatService::sendMessageAck(const string& conn_id, uint64_t msg_id, int error_code, const string& error_msg) {
    ChatMessage response;
    response.set_version(1);
    response.set_msg_type(static_cast<MsgType>(MSG_ACK));
    response.set_seq(static_cast<uint32_t>(msg_id));
    
    MsgAck ack;
    ack.set_version(1);
    ack.set_msg_type(static_cast<MsgType>(MSG_ACK));
    ack.set_seq(static_cast<uint32_t>(msg_id));
    ack.set_user_id(0);
    ack.set_ack_seq(static_cast<uint32_t>(msg_id));
    response.set_allocated_msg_ack(new MsgAck(ack));
    
    if (g_chat_server) {
        string serialized;
        response.SerializeToString(&serialized);
        g_chat_server->sendResponse(conn_id, serialized);
    }
}

/**
 * @brief 处理用户登录
 * 
 * 验证用户身份，加载离线消息和好友列表
 * 
 * @param conn_id 连接ID
 * @param msg Protobuf消息对象
 */
void ChatService::login(const string& conn_id, const ChatMessage& msg) {
    try {
        if (!msg.has_login_request()) {
            LOG_ERROR << "Login message without login_request";
            return;
        }
        
        const LoginRequest& request = msg.login_request();
        int id = request.user_id();
        string pwd = request.password();

        User user = _userModel.query(id);
        if (user.getId() == id && _userModel.verifyPassword(id, pwd)) {
            if (user.getState() == "online") {
                LOG_WARN << "User " << id << " already online";
                
                ChatMessage response;
                response.set_version(1);
                response.set_msg_type(static_cast<MsgType>(LOGIN_MSG_ACK));
                response.set_seq(msg.seq());
                
                LoginResponse login_response;
                login_response.set_version(1);
                login_response.set_msg_type(static_cast<MsgType>(LOGIN_MSG_ACK));
                login_response.set_seq(msg.seq());
                login_response.set_error_code(static_cast<ErrorCode>(ErrorCode::USER_ALREADY_ONLINE));
                login_response.set_error_msg("this account is using, input another!");
                response.set_allocated_login_response(new LoginResponse(login_response));
                
                if (g_chat_server) {
                    string serialized;
                    response.SerializeToString(&serialized);
                    g_chat_server->sendResponse(conn_id, serialized);
                }
            } else {
                // 记录用户连接关系
                {
                    lock_guard<mutex> lock(_mutex);
                    _userConnMap.insert({id, conn_id});
                }

                // 更新用户状态为在线
                user.setState("online");
                _userModel.updateState(user);

                LOG_INFO << "User " << id << " logged in";

                // 构造登录响应
                ChatMessage response;
                response.set_version(1);
                response.set_msg_type(static_cast<MsgType>(LOGIN_MSG_ACK));
                response.set_seq(msg.seq());
                
                LoginResponse login_response;
                login_response.set_version(1);
                login_response.set_msg_type(static_cast<MsgType>(LOGIN_MSG_ACK));
                login_response.set_seq(msg.seq());
                login_response.set_error_code(ErrorCode::ERROR_OK);
                login_response.set_user_id(user.getId());
                login_response.set_user_name(user.getName());

                // 加载离线消息
                vector<string> vec = _offlineMsgModel.query(id);
                if (!vec.empty()) {
                    for (const string& msg_str : vec) {
                        OfflineMessage offline_msg;
                        if (offline_msg.ParseFromString(msg_str)) {
                            login_response.add_offline_messages()->CopyFrom(offline_msg);
                        }
                    }
                    _offlineMsgModel.remove(id);
                }

                // 加载好友列表
                vector<User> userVec = _friendModel.query(id);
                if (!userVec.empty()) {
                    for (User& u : userVec) {
                        ::chat::User* friend_user = login_response.add_friends();
                        friend_user->set_id(u.getId());
                        friend_user->set_name(u.getName());
                        friend_user->set_state(u.getState() == "online" ? USER_STATE_ONLINE : USER_STATE_OFFLINE);
                    }
                }
                
                response.set_allocated_login_response(new LoginResponse(login_response));

                if (g_chat_server) {
                    string serialized;
                    response.SerializeToString(&serialized);
                    g_chat_server->sendResponse(conn_id, serialized);
                }
            }
        } else {
            LOG_WARN << "User " << id << " login failed: invalid id or password";
            
            ChatMessage response;
            response.set_version(1);
            response.set_msg_type(static_cast<MsgType>(LOGIN_MSG_ACK));
            response.set_seq(msg.seq());
            
            LoginResponse login_response;
            login_response.set_version(1);
            login_response.set_msg_type(static_cast<MsgType>(LOGIN_MSG_ACK));
            login_response.set_seq(msg.seq());
            login_response.set_error_code(static_cast<ErrorCode>(ErrorCode::INVALID_USER_OR_PASSWORD));
            login_response.set_error_msg("id or password is invalid!");
            response.set_allocated_login_response(new LoginResponse(login_response));
            
            if (g_chat_server) {
                string serialized;
                response.SerializeToString(&serialized);
                g_chat_server->sendResponse(conn_id, serialized);
            }
        }
    } catch (const exception& e) {
        LOG_ERROR << "ChatService::login exception: " << e.what();
    }
}

/**
 * @brief 处理用户注册
 * 
 * 创建新用户并返回用户ID
 * 
 * @param conn_id 连接ID
 * @param msg Protobuf消息对象
 */
void ChatService::reg(const string& conn_id, const ChatMessage& msg) {
    try {
        if (!msg.has_register_request()) {
            LOG_ERROR << "Register message without register_request";
            return;
        }
        
        const RegisterRequest& request = msg.register_request();
        string name = request.name();
        string pwd = request.password();

        if (name.empty() || pwd.empty()) {
            LOG_WARN << "Register failed: empty name or password";
            
            ChatMessage response;
            response.set_version(1);
            response.set_msg_type(static_cast<MsgType>(REG_MSG_ACK));
            response.set_seq(msg.seq());
            
            RegisterResponse register_response;
            register_response.set_version(1);
            register_response.set_msg_type(static_cast<MsgType>(REG_MSG_ACK));
            register_response.set_seq(msg.seq());
            register_response.set_error_code(static_cast<ErrorCode>(ErrorCode::INVALID_PARAM));
            register_response.set_error_msg("Name or password cannot be empty");
            response.set_allocated_register_response(new RegisterResponse(register_response));
            
            if (g_chat_server) {
                string serialized;
                response.SerializeToString(&serialized);
                g_chat_server->sendResponse(conn_id, serialized);
            }
            return;
        }

        User user;
        user.setName(name);
        user.setPwd(pwd);
        bool state = _userModel.insert(user);
        if (state) {
            LOG_INFO << "User " << user.getId() << " registered successfully";
            
            ChatMessage response;
            response.set_version(1);
            response.set_msg_type(static_cast<MsgType>(REG_MSG_ACK));
            response.set_seq(msg.seq());
            
            RegisterResponse register_response;
            register_response.set_version(1);
            register_response.set_msg_type(static_cast<MsgType>(REG_MSG_ACK));
            register_response.set_seq(msg.seq());
            register_response.set_error_code(ErrorCode::ERROR_OK);
            register_response.set_user_id(user.getId());
            response.set_allocated_register_response(new RegisterResponse(register_response));
            
            if (g_chat_server) {
                string serialized;
                response.SerializeToString(&serialized);
                g_chat_server->sendResponse(conn_id, serialized);
            }
        } else {
            LOG_WARN << "User registration failed";
            
            ChatMessage response;
            response.set_version(1);
            response.set_msg_type(static_cast<MsgType>(REG_MSG_ACK));
            response.set_seq(msg.seq());
            
            RegisterResponse register_response;
            register_response.set_version(1);
            register_response.set_msg_type(static_cast<MsgType>(REG_MSG_ACK));
            register_response.set_seq(msg.seq());
            register_response.set_error_code(static_cast<ErrorCode>(ErrorCode::ERROR_SERVER_ERROR));
            response.set_allocated_register_response(new RegisterResponse(register_response));
            
            if (g_chat_server) {
                string serialized;
                response.SerializeToString(&serialized);
                g_chat_server->sendResponse(conn_id, serialized);
            }
        }
    } catch (const exception& e) {
        LOG_ERROR << "ChatService::reg exception: " << e.what();
    }
}

/**
 * @brief 处理用户注销
 * 
 * 更新用户状态为离线
 * 
 * @param conn_id 连接ID
 * @param msg Protobuf消息对象
 */
void ChatService::loginout(const string& conn_id, const ChatMessage& msg) {
    try {
        if (!msg.has_logout_request()) {
            LOG_ERROR << "Logout message without logout_request";
            return;
        }
        
        const LogoutRequest& request = msg.logout_request();
        int userid = request.user_id();

        {
            lock_guard<mutex> lock(_mutex);
            auto it = _userConnMap.find(userid);
            if (it != _userConnMap.end()) {
                _userConnMap.erase(it);
            }
        }

        User user(userid, "", "", "offline");
        _userModel.updateState(user);

        LOG_INFO << "User " << userid << " logged out";

        ChatMessage response;
        response.set_version(1);
        response.set_msg_type(static_cast<MsgType>(MSG_ACK));
        response.set_seq(msg.seq());
        
        MsgAck ack;
        ack.set_version(1);
        ack.set_msg_type(static_cast<MsgType>(MSG_ACK));
        ack.set_seq(msg.seq());
        ack.set_user_id(userid);
        ack.set_ack_seq(msg.seq());
        response.set_allocated_msg_ack(new MsgAck(ack));
        
        if (g_chat_server) {
            string serialized;
            response.SerializeToString(&serialized);
            g_chat_server->sendResponse(conn_id, serialized);
        }
    } catch (const exception& e) {
        LOG_ERROR << "ChatService::loginout exception: " << e.what();
    }
}

/**
 * @brief 处理单聊消息
 * 
 * 如果接收方在线则直接发送，否则存储为离线消息
 * 
 * @param conn_id 连接ID
 * @param msg Protobuf消息对象
 */
void ChatService::oneChat(const string& conn_id, const ChatMessage& msg) {
    try {
        if (!msg.has_one_chat_request()) {
            LOG_ERROR << "OneChat message without one_chat_request";
            return;
        }
        
        const OneChatRequest& request = msg.one_chat_request();
        int toid = request.to_id();

        {
            lock_guard<mutex> lock(_mutex);
            auto it = _userConnMap.find(toid);
            if (it != _userConnMap.end()) {
                LOG_DEBUG << "User " << toid << " is online, sending message";
                
                if (g_chat_server) {
                    string serialized;
                    msg.SerializeToString(&serialized);
                    g_chat_server->sendResponse(it->second, serialized);
                }
                
                sendMessageAck(conn_id, msg.seq(), 0, "");
                return;
            }
        }

        // 存储离线消息
        string serialized_msg;
        msg.SerializeToString(&serialized_msg);
        _offlineMsgModel.insert(toid, serialized_msg);
        
        LOG_INFO << "Message to " << toid << " saved as offline message";
        sendMessageAck(conn_id, msg.seq(), 0, "Message saved");
    } catch (const exception& e) {
        LOG_ERROR << "ChatService::oneChat exception: " << e.what();
        sendMessageAck(conn_id, 0, static_cast<int>(ErrorCode::ERROR_SERVER_ERROR), "System error");
    }
}

/**
 * @brief 处理添加好友
 * 
 * @param conn_id 连接ID
 * @param msg Protobuf消息对象
 */
void ChatService::addFriend(const string& conn_id, const ChatMessage& msg) {
    try {
        if (!msg.has_add_friend_request()) {
            LOG_ERROR << "AddFriend message without add_friend_request";
            return;
        }
        
        const AddFriendRequest& request = msg.add_friend_request();
        int userid = request.user_id();
        int friendid = request.friend_id();

        if (userid == friendid) {
            LOG_WARN << "Cannot add self as friend";
            
            ChatMessage response;
            response.set_version(1);
            response.set_msg_type(static_cast<MsgType>(MSG_ACK));
            response.set_seq(msg.seq());
            
            MsgAck ack;
            ack.set_version(1);
            ack.set_msg_type(static_cast<MsgType>(MSG_ACK));
            ack.set_seq(msg.seq());
            ack.set_user_id(userid);
            ack.set_ack_seq(msg.seq());
            response.set_allocated_msg_ack(new MsgAck(ack));
            
            if (g_chat_server) {
                string serialized;
                response.SerializeToString(&serialized);
                g_chat_server->sendResponse(conn_id, serialized);
            }
            return;
        }

        _friendModel.insert(userid, friendid);
        LOG_INFO << "User " << userid << " added friend " << friendid;

        ChatMessage response;
        response.set_version(1);
        response.set_msg_type(static_cast<MsgType>(MSG_ACK));
        response.set_seq(msg.seq());
        
        MsgAck ack;
        ack.set_version(1);
        ack.set_msg_type(static_cast<MsgType>(MSG_ACK));
        ack.set_seq(msg.seq());
        ack.set_user_id(userid);
        ack.set_ack_seq(msg.seq());
        response.set_allocated_msg_ack(new MsgAck(ack));
        
        if (g_chat_server) {
            string serialized;
            response.SerializeToString(&serialized);
            g_chat_server->sendResponse(conn_id, serialized);
        }
    } catch (const exception& e) {
        LOG_ERROR << "ChatService::addFriend exception: " << e.what();
        
        ChatMessage response;
        response.set_version(1);
        response.set_msg_type(static_cast<MsgType>(MSG_ACK));
        response.set_seq(msg.seq());
        
        MsgAck ack;
        ack.set_version(1);
        ack.set_msg_type(static_cast<MsgType>(MSG_ACK));
        ack.set_seq(msg.seq());
        ack.set_user_id(0);
        ack.set_ack_seq(msg.seq());
        response.set_allocated_msg_ack(new MsgAck(ack));
        
        if (g_chat_server) {
            string serialized;
            response.SerializeToString(&serialized);
            g_chat_server->sendResponse(conn_id, serialized);
        }
    }
}

/**
 * @brief 处理心跳消息
 * 
 * 更新用户心跳时间，返回心跳响应
 * 
 * @param conn_id 连接ID
 * @param msg Protobuf消息对象
 */
void ChatService::handleHeartbeat(const string& conn_id, const ChatMessage& msg) {
    try {
        if (!msg.has_heartbeat_request()) {
            LOG_ERROR << "Heartbeat message without heartbeat_request";
            return;
        }
        
        const HeartbeatRequest& request = msg.heartbeat_request();
        int userid = request.user_id();
        LOG_DEBUG << "Received heartbeat from user: " << userid;

        ChatMessage response;
        response.set_version(1);
        response.set_msg_type(static_cast<MsgType>(HEARTBEAT_ACK));
        response.set_seq(msg.seq());
        
        HeartbeatResponse heartbeat_response;
        heartbeat_response.set_version(1);
        heartbeat_response.set_msg_type(static_cast<MsgType>(HEARTBEAT_ACK));
        heartbeat_response.set_seq(msg.seq());
        heartbeat_response.set_error_code(ErrorCode::ERROR_OK);
        response.set_allocated_heartbeat_response(new HeartbeatResponse(heartbeat_response));
        
        if (g_chat_server) {
            string serialized;
            response.SerializeToString(&serialized);
            g_chat_server->sendResponse(conn_id, serialized);
        }
    } catch (const exception& e) {
        LOG_ERROR << "ChatService::handleHeartbeat exception: " << e.what();
    }
}
