#include "config.h"
#include "logging.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace chat {
namespace util {

Config::Config() : _server{"127.0.0.1", 6000, 30, 1000},
                   _mysql{"127.0.0.1", 3306, "root", "", "chat_db", 10},
                   _redis{"127.0.0.1", 6379, "", 0},
                   _log{"logs", "info", 10485760, 5} {}

Config& Config::instance() {
    static Config instance;
    return instance;
}

bool Config::load(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        LOG_ERROR << "Config::load - Failed to open config file: " << file_path;
        return false;
    }
    
    std::string line;
    std::string current_section;
    
    while (std::getline(file, line)) {
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        if (line[0] == '[' && line.back() == ']') {
            current_section = line.substr(1, line.size() - 2);
            continue;
        }
        
        if (!current_section.empty()) {
            parse_key_value(line, current_section);
        }
    }
    
    return validate();
}

bool Config::parse_key_value(const std::string& line, const std::string& section) {
    size_t eq_pos = line.find('=');
    if (eq_pos == std::string::npos) {
        return false;
    }
    
    std::string key = line.substr(0, eq_pos);
    std::string value = line.substr(eq_pos + 1);
    
    key.erase(key.find_last_not_of(" \t") + 1);
    key.erase(0, key.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);
    value.erase(0, value.find_first_not_of(" \t"));
    
    _raw_config[section][key] = value;
    
    if (section == "server") {
        if (key == "ip") _server.ip = value;
        else if (key == "port") _server.port = std::stoi(value);
        else if (key == "heartbeat_timeout") _server.heartbeat_timeout = std::stoi(value);
        else if (key == "max_connections") _server.max_connections = std::stoi(value);
    } else if (section == "mysql") {
        if (key == "host") _mysql.host = value;
        else if (key == "port") _mysql.port = std::stoi(value);
        else if (key == "user") _mysql.user = value;
        else if (key == "password") _mysql.password = value;
        else if (key == "database") _mysql.database = value;
        else if (key == "max_connections") _mysql.max_connections = std::stoi(value);
    } else if (section == "redis") {
        if (key == "host") _redis.host = value;
        else if (key == "port") _redis.port = std::stoi(value);
        else if (key == "password") _redis.password = value;
        else if (key == "db") _redis.db = std::stoi(value);
    } else if (section == "logging") {
        if (key == "log_dir") _log.log_dir = value;
        else if (key == "log_level") _log.log_level = value;
        else if (key == "max_file_size") _log.max_file_size = std::stoull(value);
        else if (key == "max_files") _log.max_files = std::stoi(value);
    }
    
    return true;
}

void Config::load_from_env() {
    if (const char* env = std::getenv("MYSQL_HOST")) _mysql.host = env;
    if (const char* env = std::getenv("MYSQL_PORT")) _mysql.port = std::stoi(env);
    if (const char* env = std::getenv("MYSQL_USER")) _mysql.user = env;
    if (const char* env = std::getenv("MYSQL_PASSWORD")) _mysql.password = env;
    if (const char* env = std::getenv("MYSQL_DATABASE")) _mysql.database = env;
    
    if (const char* env = std::getenv("REDIS_HOST")) _redis.host = env;
    if (const char* env = std::getenv("REDIS_PORT")) _redis.port = std::stoi(env);
    
    LOG_INFO << "Config::load_from_env - Loaded environment variables";
}

bool Config::validate() const {
    bool valid = true;
    
    if (_server.ip.empty()) {
        LOG_ERROR << "Config::validate - Server IP is empty";
        valid = false;
    }
    if (_server.port <= 0 || _server.port > 65535) {
        LOG_ERROR << "Config::validate - Invalid server port: " << _server.port;
        valid = false;
    }
    
    if (_mysql.host.empty()) {
        LOG_ERROR << "Config::validate - MySQL host is empty";
        valid = false;
    }
    if (_mysql.port <= 0 || _mysql.port > 65535) {
        LOG_ERROR << "Config::validate - Invalid MySQL port: " << _mysql.port;
        valid = false;
    }
    if (_mysql.user.empty()) {
        LOG_ERROR << "Config::validate - MySQL user is empty";
        valid = false;
    }
    if (_mysql.database.empty()) {
        LOG_ERROR << "Config::validate - MySQL database is empty";
        valid = false;
    }
    
    if (_redis.host.empty()) {
        LOG_ERROR << "Config::validate - Redis host is empty";
        valid = false;
    }
    if (_redis.port <= 0 || _redis.port > 65535) {
        LOG_ERROR << "Config::validate - Invalid Redis port: " << _redis.port;
        valid = false;
    }
    
    if (_log.log_dir.empty()) {
        LOG_ERROR << "Config::validate - Log directory is empty";
        valid = false;
    }
    
    if (valid) {
        LOG_INFO << "Config::validate - Configuration validation passed";
    }
    
    return valid;
}

} // namespace util
} // namespace chat
