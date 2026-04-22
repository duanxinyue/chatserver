#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <chrono>

#include "net/TcpServer.h"

namespace chat {

struct ServerConfig {
    std::string ip;
    int port;
    int heartbeat_timeout;
    int max_connections;
    
    ServerConfig() 
        : port(6000), heartbeat_timeout(30), max_connections(1000) {}
};

class ChatServer {
public:
    ChatServer();
    ~ChatServer();
    
    bool init(const ServerConfig& config);
    bool start();
    void stop();
    
    void updateHeartbeat(int user_id);
    bool isUserOnline(int user_id);
    void markUserOffline(int user_id);
    
    uint64_t generateMessageId();
    
    void sendResponse(const std::string& conn_id, const std::string& message);
    
private:
    void heartbeatCheckLoop();
    void onConnection(const std::string& conn_id);
    void onMessage(const std::string& conn_id, const std::string& message);
    void onClose(const std::string& conn_id);
    
    std::unique_ptr<muduo::net::EventLoop> _loop;
    std::unique_ptr<net::TcpServer> _server;
    ServerConfig _config;
    std::atomic<bool> _running;
    
    struct ConnectionState {
        int user_id = -1;
        std::chrono::steady_clock::time_point last_heartbeat;
        bool authenticated = false;
    };
    
    mutable std::mutex _conn_mutex;
    std::unordered_map<std::string, ConnectionState> _conn_states;
    std::unordered_map<int, std::string> _user_conn_map;
    
    std::atomic<uint64_t> _msg_id_counter;
};

} // namespace chat

#endif // CHATSERVER_H
