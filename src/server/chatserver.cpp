#include "chatserver.hpp"
#include "chatservice.hpp"
#include "logging.h"
#include "chat.pb.h"
#include <thread>
#include <muduo/net/InetAddress.h>

namespace chat {

/**
 * @brief ChatServer 构造函数
 * 
 * 初始化服务器状态和消息ID计数器
 */
ChatServer::ChatServer() : _running(false), _msg_id_counter(0) {}

/**
 * @brief ChatServer 析构函数
 * 
 * 停止服务器并释放资源
 */
ChatServer::~ChatServer() {
    stop();
}

/**
 * @brief 初始化服务器
 * 
 * @param config 服务器配置参数
 * @return true 初始化成功，false 失败
 */
bool ChatServer::init(const ServerConfig& config) {
    _config = config;
    _loop = std::make_unique<muduo::net::EventLoop>();
    
    muduo::net::InetAddress listen_addr(_config.port);
    
    _server = std::make_unique<net::TcpServer>(
        _loop.get(), 
        listen_addr, 
        "ChatServer"
    );
    
    // 设置回调函数
    _server->setConnectionCallback(
        std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
    _server->setMessageCallback(
        std::bind(&ChatServer::onMessage, this, std::placeholders::_1, 
                  std::placeholders::_2));
    _server->setCloseCallback(
        std::bind(&ChatServer::onClose, this, std::placeholders::_1));
    
    return true;
}

/**
 * @brief 启动服务器
 * 
 * 创建心跳检测线程，启动TcpServer，开始事件循环
 * 
 * @return true 启动成功，false 失败
 */
bool ChatServer::start() {
    _running = true;
    
    // 启动心跳检测线程（后台运行）
    std::thread heartbeat_thread(&ChatServer::heartbeatCheckLoop, this);
    heartbeat_thread.detach();
    
    // 启动TCP服务器
    _server->start();
    
    LOG_INFO << "ChatServer started on " << _config.ip << ":" << _config.port;
    
    // 进入事件循环
    _loop->loop();
    
    return true;
}

/**
 * @brief 停止服务器
 * 
 * 停止事件循环，释放资源
 */
void ChatServer::stop() {
    _running = false;
    if (_loop) {
        _loop->quit();
    }
    LOG_INFO << "ChatServer stopped";
}

/**
 * @brief 心跳检测循环
 * 
 * 定期检查客户端连接状态，超时未响应则标记为离线
 */
void ChatServer::heartbeatCheckLoop() {
    while (_running) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        std::lock_guard<std::mutex> lock(_conn_mutex);
        auto now = std::chrono::steady_clock::now();
        
        for (auto it = _conn_states.begin(); it != _conn_states.end(); ) {
            auto& state = it->second;
            if (state.authenticated && state.user_id != -1) {
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    now - state.last_heartbeat).count();
                if (elapsed > _config.heartbeat_timeout) {
                    LOG_WARN << "User " << state.user_id << " heartbeat timeout, marking offline";
                    _user_conn_map.erase(state.user_id);
                    it = _conn_states.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }
}

/**
 * @brief 处理新连接
 * 
 * @param conn_id 连接ID
 */
void ChatServer::onConnection(const std::string& conn_id) {
    LOG_DEBUG << "New connection: " << conn_id;
    {
        std::lock_guard<std::mutex> lock(_conn_mutex);
        _conn_states[conn_id] = ConnectionState();
    }
}

/**
 * @brief 处理收到的消息
 * 
 * 使用Protobuf解析消息，更新心跳时间，然后交给ChatBusinessService处理
 * 
 * @param conn_id 连接ID
 * @param message 消息内容（Protobuf序列化数据）
 */
void ChatServer::onMessage(const std::string& conn_id, const std::string& message) {
    try {
        // 使用Protobuf解析消息
        ChatMessage msg;
        if (!msg.ParseFromString(message)) {
            LOG_ERROR << "Failed to parse ChatMessage from connection: " << conn_id;
            return;
        }
        
        // 更新心跳时间
        {
            std::lock_guard<std::mutex> lock(_conn_mutex);
            auto it = _conn_states.find(conn_id);
            if (it != _conn_states.end()) {
                it->second.last_heartbeat = std::chrono::steady_clock::now();
            }
        }
        
        // 交给业务层处理
        ChatBusinessService::instance()->handleMessage(conn_id, msg);
        
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to process message from " << conn_id << ": " << e.what();
    }
}

void ChatServer::onClose(const std::string& conn_id) {
    LOG_DEBUG << "Connection closed: " << conn_id;
    {
        std::lock_guard<std::mutex> lock(_conn_mutex);
        auto it = _conn_states.find(conn_id);
        if (it != _conn_states.end()) {
            if (it->second.user_id != -1) {
                _user_conn_map.erase(it->second.user_id);
            }
            _conn_states.erase(it);
        }
    }
}

void ChatServer::updateHeartbeat(int user_id) {
    std::lock_guard<std::mutex> lock(_conn_mutex);
    for (auto& pair : _conn_states) {
        if (pair.second.user_id == user_id) {
            pair.second.last_heartbeat = std::chrono::steady_clock::now();
            break;
        }
    }
}

bool ChatServer::isUserOnline(int user_id) {
    std::lock_guard<std::mutex> lock(_conn_mutex);
    return _user_conn_map.find(user_id) != _user_conn_map.end();
}

void ChatServer::markUserOffline(int user_id) {
    std::lock_guard<std::mutex> lock(_conn_mutex);
    auto it = _user_conn_map.find(user_id);
    if (it != _user_conn_map.end()) {
        _conn_states.erase(it->second);
        _user_conn_map.erase(it);
    }
}

uint64_t ChatServer::generateMessageId() {
    return ++_msg_id_counter;
}

void ChatServer::sendResponse(const std::string& conn_id, const std::string& message) {
    if (_server) {
        _server->send(conn_id, message);
    }
}

} // namespace chat
