#include "TcpServer.h"
#include "MessageCodec.h"
#include "logging.h"

using namespace muduo;
using namespace muduo::net;

namespace chat {
namespace net {

TcpServer::TcpServer(EventLoop* loop, 
                     const InetAddress& listenAddr,
                     const std::string& nameArg)
    : server_(new muduo::net::TcpServer(loop, listenAddr, nameArg)) {
    server_->setConnectionCallback(
        std::bind(&TcpServer::onConnection, this, std::placeholders::_1));
    server_->setMessageCallback(
        std::bind(&TcpServer::onMessage, this, std::placeholders::_1, 
                  std::placeholders::_2, std::placeholders::_3));
}

TcpServer::~TcpServer() {
}

void TcpServer::start() {
    LOG_INFO << "TcpServer starting on " << server_->ipPort();
    server_->start();
}

void TcpServer::onConnection(const TcpConnectionPtr& conn) {
    std::string conn_id = conn->name();
    
    if (conn->connected()) {
        LOG_INFO << "New connection: " << conn_id;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            connections_[conn_id] = conn;
        }
        if (connection_callback_) {
            connection_callback_(conn_id);
        }
    } else {
        LOG_INFO << "Connection closed: " << conn_id;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            connections_.erase(conn_id);
        }
        if (close_callback_) {
            close_callback_(conn_id);
        }
    }
}

void TcpServer::onMessage(const TcpConnectionPtr& conn,
                          Buffer* buf,
                          Timestamp receiveTime) {
    std::string conn_id = conn->name();
    
    while (buf->readableBytes() >= MessageCodec::kHeaderLen) {
        std::string message;
        if (MessageCodec::decode(buf, message)) {
            if (message_callback_) {
                message_callback_(conn_id, message);
            }
        } else {
            LOG_ERROR << "Invalid message from " << conn_id;
            conn->shutdown();
            break;
        }
    }
}

void TcpServer::send(const std::string& conn_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = connections_.find(conn_id);
    if (it != connections_.end()) {
        Buffer buf;
        MessageCodec::encode(message, &buf);
        it->second->send(&buf);
    }
}

void TcpServer::sendToAll(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    Buffer buf;
    MessageCodec::encode(message, &buf);
    for (auto& pair : connections_) {
        pair.second->send(&buf);
    }
}

} // namespace net
} // namespace chat
