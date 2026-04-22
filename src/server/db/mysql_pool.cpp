#include "mysql_pool.h"
#include "logging.h"
#include <cstring>

namespace chat {
namespace db {

MySQLPool::MySQLPool() 
    : _port(3306), _max_connections(10), _idle_timeout(60), _health_check_interval(30) {}

MySQLPool::~MySQLPool() {
    std::lock_guard<std::mutex> lock(_mutex);
    while (!_idle_conns.empty()) {
        MYSQL* conn = _idle_conns.front();
        _idle_conns.pop();
        mysql_close(conn);
    }
}

MySQLPool& MySQLPool::instance() {
    static MySQLPool pool;
    return pool;
}

bool MySQLPool::init(const std::string& host, int port, 
                     const std::string& user, const std::string& password, 
                     const std::string& database, int max_connections,
                     int idle_timeout, int health_check_interval) {
    _host = host;
    _port = port;
    _user = user;
    _password = password;
    _database = database;
    _max_connections = max_connections;
    _idle_timeout = idle_timeout;
    _health_check_interval = health_check_interval;
    
    for (int i = 0; i < _max_connections; ++i) {
        MYSQL* conn = create_connection();
        if (conn) {
            _idle_conns.push(conn);
        }
    }
    
    LOG_INFO << "MySQL pool initialized with " << _idle_conns.size() << " connections";
    return !_idle_conns.empty();
}

MYSQL* MySQLPool::create_connection() {
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        LOG_ERROR << "mysql_init failed";
        return nullptr;
    }
    
    mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");
    bool reconnect = true;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
    
    if (!mysql_real_connect(conn, _host.c_str(), _user.c_str(), 
                           _password.c_str(), _database.c_str(), 
                           _port, nullptr, 0)) {
        LOG_ERROR << "mysql_real_connect failed: " << mysql_error(conn);
        mysql_close(conn);
        return nullptr;
    }
    
    return conn;
}

bool MySQLPool::is_connection_valid(MYSQL* conn) const {
    if (!conn) return false;
    
    if (mysql_ping(conn) != 0) {
        LOG_WARN << "MySQL connection is invalid: " << mysql_error(conn);
        return false;
    }
    
    return true;
}

std::unique_ptr<MYSQL, std::function<void(MYSQL*)>> MySQLPool::get_connection() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    while (!_idle_conns.empty()) {
        MYSQL* conn = _idle_conns.front();
        _idle_conns.pop();
        
        if (is_connection_valid(conn)) {
            return std::unique_ptr<MYSQL, std::function<void(MYSQL*)>>(conn, 
                [this](MYSQL* c) { this->release_connection(c); });
        } else {
            LOG_WARN << "Found invalid connection, creating new one";
            mysql_close(conn);
        }
    }
    
    if (_idle_conns.size() < (size_t)_max_connections) {
        MYSQL* conn = create_connection();
        if (conn) {
            return std::unique_ptr<MYSQL, std::function<void(MYSQL*)>>(conn, 
                [this](MYSQL* c) { this->release_connection(c); });
        }
    }
    
    LOG_WARN << "MySQL pool exhausted";
    return nullptr;
}

void MySQLPool::release_connection(MYSQL* conn) {
    if (!conn) return;
    
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (is_connection_valid(conn)) {
        if (_idle_conns.size() < (size_t)_max_connections) {
            _idle_conns.push(conn);
        } else {
            mysql_close(conn);
        }
    } else {
        LOG_WARN << "Releasing invalid connection";
        mysql_close(conn);
    }
}

} // namespace db
} // namespace chat
