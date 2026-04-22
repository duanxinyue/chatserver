#ifndef MYSQL_POOL_H
#define MYSQL_POOL_H

#include <mysql/mysql.h>
#include <string>
#include <mutex>
#include <queue>
#include <memory>
#include <functional>

namespace chat {
namespace db {

class MySQLPool {
public:
    ~MySQLPool();
    
    static MySQLPool& instance();
    
    bool init(const std::string& host, int port, 
              const std::string& user, const std::string& password, 
              const std::string& database, int max_connections = 10,
              int idle_timeout = 60, int health_check_interval = 30);
    
    std::unique_ptr<MYSQL, std::function<void(MYSQL*)>> get_connection();
    
    void release_connection(MYSQL* conn);
    
    bool is_connection_valid(MYSQL* conn) const;
    
private:
    MySQLPool();
    
    MySQLPool(const MySQLPool&) = delete;
    MySQLPool& operator=(const MySQLPool&) = delete;
    
    MYSQL* create_connection();
    
    std::string _host;
    int _port;
    std::string _user;
    std::string _password;
    std::string _database;
    int _max_connections;
    int _idle_timeout;
    int _health_check_interval;
    
    std::queue<MYSQL*> _idle_conns;
    std::mutex _mutex;
};

class MySQLConnectionGuard {
public:
    MySQLConnectionGuard(MySQLPool& pool) 
        : _pool(pool), _conn(nullptr) {
        _conn = _pool.get_connection().release();
    }
    
    ~MySQLConnectionGuard() {
        if (_conn) {
            _pool.release_connection(_conn);
        }
    }
    
    MYSQL* get() const { return _conn; }
    
    operator bool() const { return _conn != nullptr; }
    
private:
    MySQLPool& _pool;
    MYSQL* _conn;
};

} // namespace db
} // namespace chat

#endif // MYSQL_POOL_H
