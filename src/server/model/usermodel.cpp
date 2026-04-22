#include "usermodel.hpp"
#include "db/mysql_pool.h"
#include "util/crypto.h"
#include "util/logging.h"
using namespace std;
using namespace chat::db;

bool UserModel::insert(User& user) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return false;
    }
    
    // 对密码进行哈希加密
    string hashed_pwd = chat::util::Crypto::hash_password(user.getPwd());
    
    // 使用预编译语句防止SQL注入
    const char* sql = "INSERT INTO user(name, password, state) VALUES(?, ?, ?)";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return false;
    }
    
    // 绑定参数
    MYSQL_BIND params[3];
    memset(params, 0, sizeof(params));
    
    string name = user.getName();
    string state = user.getState();
    
    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = const_cast<char*>(name.c_str());
    params[0].buffer_length = name.length();
    
    params[1].buffer_type = MYSQL_TYPE_STRING;
    params[1].buffer = const_cast<char*>(hashed_pwd.c_str());
    params[1].buffer_length = hashed_pwd.length();
    
    params[2].buffer_type = MYSQL_TYPE_STRING;
    params[2].buffer = const_cast<char*>(state.c_str());
    params[2].buffer_length = state.length();
    
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return false;
    }
    
    // 获取插入成功的用户数据生成的主键id
    user.setId(mysql_stmt_insert_id(stmt));
    mysql_stmt_close(stmt);
    
    LOG_INFO << "User " << user.getName() << " registered with id " << user.getId();
    return true;
}

User UserModel::query(int id) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return User();
    }
    
    const char* sql = "SELECT id, name, password, state FROM user WHERE id = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return User();
    }
    
    MYSQL_BIND params[1];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &id;
    
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return User();
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return User();
    }
    
    // 绑定结果
    MYSQL_BIND result[4];
    memset(result, 0, sizeof(result));
    
    int result_id = 0;
    char result_name[50] = {0};
    char result_pwd[256] = {0};
    char result_state[20] = {0};
    
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &result_id;
    
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = result_name;
    result[1].buffer_length = sizeof(result_name);
    
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = result_pwd;
    result[2].buffer_length = sizeof(result_pwd);
    
    result[3].buffer_type = MYSQL_TYPE_STRING;
    result[3].buffer = result_state;
    result[3].buffer_length = sizeof(result_state);
    
    if (mysql_stmt_bind_result(stmt, result) != 0) {
        LOG_ERROR << "mysql_stmt_bind_result failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return User();
    }
    
    User user;
    if (mysql_stmt_fetch(stmt) == 0) {
        user.setId(result_id);
        user.setName(result_name);
        user.setPwd(result_pwd);
        user.setState(result_state);
    }
    
    mysql_stmt_close(stmt);
    return user;
}

bool UserModel::updateState(User user) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return false;
    }
    
    const char* sql = "UPDATE user SET state = ? WHERE id = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return false;
    }
    
    MYSQL_BIND params[2];
    memset(params, 0, sizeof(params));
    
    string state = user.getState();
    
    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = const_cast<char*>(state.c_str());
    params[0].buffer_length = state.length();
    
    int id = user.getId();
    params[1].buffer_type = MYSQL_TYPE_LONG;
    params[1].buffer = &id;
    
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return false;
    }
    
    mysql_stmt_close(stmt);
    return true;
}

void UserModel::resetState() {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return;
    }
    
    const char* sql = "UPDATE user SET state = 'offline' WHERE state = 'online'";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return;
    }
    
    if (mysql_stmt_prepare(stmt, sql, strlen(sql)) != 0) {
        LOG_ERROR << "mysql_stmt_prepare failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt);
    }
    
    mysql_stmt_close(stmt);
}

bool UserModel::verifyPassword(int id, const string& password) {
    User user = query(id);
    if (user.getId() == -1) {
        return false;
    }
    return chat::util::Crypto::verify_password(password, user.getPwd());
}

string UserModel::getState(int id) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return "offline";
    }
    
    const char* sql = "SELECT state FROM user WHERE id = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return "offline";
    }
    
    MYSQL_BIND params[1];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &id;
    
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return "offline";
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return "offline";
    }
    
    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));
    
    char result_state[20] = {0};
    result[0].buffer_type = MYSQL_TYPE_STRING;
    result[0].buffer = result_state;
    result[0].buffer_length = sizeof(result_state);
    
    if (mysql_stmt_bind_result(stmt, result) != 0) {
        LOG_ERROR << "mysql_stmt_bind_result failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return "offline";
    }
    
    string state = "offline";
    if (mysql_stmt_fetch(stmt) == 0) {
        state = result_state;
    }
    
    mysql_stmt_close(stmt);
    return state;
}
