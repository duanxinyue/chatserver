#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace chat {
namespace util {

struct ServerConfig {
    std::string ip;
    int port;
    int heartbeat_timeout;
    int max_connections;
};

struct MySQLConfig {
    std::string host;
    int port;
    std::string user;
    std::string password;
    std::string database;
    int max_connections;
};

struct RedisConfig {
    std::string host;
    int port;
    std::string password;
    int db;
};

struct LogConfig {
    std::string log_dir;
    std::string log_level;
    size_t max_file_size;
    int max_files;
};

class Config {
public:
    static Config& instance();
    
    bool load(const std::string& file_path);
    
    void load_from_env();
    
    bool validate() const;
    
    const ServerConfig& get_server_config() const { return _server; }
    const MySQLConfig& get_mysql_config() const { return _mysql; }
    const RedisConfig& get_redis_config() const { return _redis; }
    const LogConfig& get_log_config() const { return _log; }
    
private:
    Config();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    bool parse_section(const std::string& line, std::string& current_section);
    bool parse_key_value(const std::string& line, const std::string& section);
    
    ServerConfig _server;
    MySQLConfig _mysql;
    RedisConfig _redis;
    LogConfig _log;
    
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> _raw_config;
};

} // namespace util
} // namespace chat

#endif // CONFIG_H
