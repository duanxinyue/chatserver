#include <gtest/gtest.h>
#include "chatserver.hpp"
#include <thread>
#include <chrono>

using namespace chat;

TEST(ChatServerTest, ServerInitAndStart) {
    ServerConfig config;
    config.ip = "127.0.0.1";
    config.port = 6666;
    config.heartbeat_timeout = 30;
    config.max_connections = 100;
    
    std::unique_ptr<ChatServer> server = std::make_unique<ChatServer>();
    bool init_result = server->init(config);
    
    EXPECT_TRUE(init_result);
}

TEST(ChatServerTest, MessageIdGeneration) {
    ServerConfig config;
    config.ip = "127.0.0.1";
    config.port = 6667;
    
    std::unique_ptr<ChatServer> server = std::make_unique<ChatServer>();
    server->init(config);
    
    uint64_t id1 = server->generateMessageId();
    uint64_t id2 = server->generateMessageId();
    uint64_t id3 = server->generateMessageId();
    
    EXPECT_EQ(id2, id1 + 1);
    EXPECT_EQ(id3, id2 + 1);
}

TEST(ChatServerTest, HeartbeatTimeoutConfig) {
    ServerConfig config;
    config.ip = "127.0.0.1";
    config.port = 6668;
    config.heartbeat_timeout = 5;
    
    std::unique_ptr<ChatServer> server = std::make_unique<ChatServer>();
    bool init_result = server->init(config);
    
    EXPECT_TRUE(init_result);
}

TEST(ChatServerTest, OnlineStatusManagement) {
    ServerConfig config;
    config.ip = "127.0.0.1";
    config.port = 6669;
    
    std::unique_ptr<ChatServer> server = std::make_unique<ChatServer>();
    server->init(config);
    
    EXPECT_FALSE(server->isUserOnline(1));
    
    server->markUserOffline(1);
    EXPECT_FALSE(server->isUserOnline(1));
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
