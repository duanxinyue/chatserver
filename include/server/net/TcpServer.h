#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <mutex>

namespace chat {
namespace net {

class TcpServer {
public:
    using ConnectionCallback = std::function<void(const std::string& conn_id)>;
    using MessageCallback = std::function<void(const std::string& conn_id, const std::string& message)>;
    using CloseCallback = std::function<void(const std::string& conn_id)>;
    
    TcpServer(muduo::net::EventLoop* loop, 
              const muduo::net::InetAddress& listenAddr,
              const std::string& nameArg);
    
    ~TcpServer();
    
    void setConnectionCallback(const ConnectionCallback& cb) { connection_callback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { message_callback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { close_callback_ = cb; }
    
    void start();
    
    void send(const std::string& conn_id, const std::string& message);
    void sendToAll(const std::string& message);
    
private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn);
    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp receiveTime);
    
    std::unique_ptr<muduo::net::TcpServer> server_;
    std::unordered_map<std::string, muduo::net::TcpConnectionPtr> connections_;
    
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    CloseCallback close_callback_;
    
    mutable std::mutex mutex_;
};

} // namespace net
} // namespace chat

#endif // TCPSERVER_H
