#include "chatserver.hpp"
#include "chatservice.hpp"
#include "logging.h"
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
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }

    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    chat::ServerConfig config;
    config.ip = ip;
    config.port = port;
    config.heartbeat_timeout = 30;
    config.max_connections = 1000;

    std::unique_ptr<chat::ChatServer> server = std::make_unique<chat::ChatServer>();
    g_chat_server = server.get();
    
    if (!server->init(config)) {
        cerr << "Failed to initialize ChatServer" << endl;
        exit(-1);
    }
    
    if (!server->start()) {
        cerr << "Failed to start ChatServer" << endl;
        exit(-1);
    }

    return 0;
}
