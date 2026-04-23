#include "chatserver.hpp"
#include "chatservice.hpp"
#include "logging.h"
#include "util/config.h"
#include <iostream>
#include <signal.h>
#include <memory>
#include <string>
#include <thread>

using namespace std;

chat::ChatServer* g_chat_server = nullptr;

void handle_sigint(int sig) {
    LOG_INFO << "Received signal " << sig << ", shutting down...";
    if (g_chat_server) {
        g_chat_server->stop();
    }
    exit(0);
}

int main(int argc, char** argv) {
    signal(SIGINT, handle_sigint);
    
    if (argc < 3) {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000 [config_path]" << endl;
        exit(-1);
    }

    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);
    
    std::string config_path = "config/server.conf";
    if (argc >= 4) {
        config_path = argv[3];
    }

    auto& config = chat::util::Config::instance();
    if (!config.load(config_path)) {
        LOG_WARN << "Failed to load config file: " << config_path << ", using defaults";
    }
    config.load_from_env();

    chat::ServerConfig server_config;
    server_config.ip = ip;
    server_config.port = port;
    server_config.heartbeat_timeout = config.get_server_config().heartbeat_timeout;
    server_config.max_connections = config.get_server_config().max_connections;

    auto& redis_config = config.get_redis_config();
    chat::ChatBusinessService::instance()->initRedis(redis_config.host, redis_config.port);

    std::unique_ptr<chat::ChatServer> server = std::make_unique<chat::ChatServer>();
    g_chat_server = server.get();
    
    if (!server->init(server_config)) {
        cerr << "Failed to initialize ChatServer" << endl;
        exit(-1);
    }
    
    if (!server->start()) {
        cerr << "Failed to start ChatServer" << endl;
        exit(-1);
    }

    return 0;
}
