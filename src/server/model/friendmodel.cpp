#include "friendmodel.hpp"
#include "db/mysql_pool.h"
#include "util/logging.h"
using namespace std;
using namespace chat::db;

bool FriendModel::insert(int userid, int friendid) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return false;
    }
    
    const char* sql = "INSERT INTO friend(userid, friendid) VALUES(?, ?)";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return false;
    }
    
    MYSQL_BIND params[2];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &userid;
    
    params[1].buffer_type = MYSQL_TYPE_LONG;
    params[1].buffer = &friendid;
    
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
    LOG_INFO << "Friend relationship created: " << userid << " <-> " << friendid;
    return true;
}

vector<User> FriendModel::query(int userid) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return {};
    }
    
    const char* sql = "SELECT a.id, a.name, a.state FROM user a INNER JOIN friend b ON b.friendid = a.id WHERE b.userid = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return {};
    }
    
    MYSQL_BIND params[1];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &userid;
    
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return {};
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return {};
    }
    
    MYSQL_BIND result[3];
    memset(result, 0, sizeof(result));
    
    int id = 0;
    char name[50] = {0};
    char state[20] = {0};
    
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &id;
    
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = name;
    result[1].buffer_length = sizeof(name);
    
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = state;
    result[2].buffer_length = sizeof(state);
    
    if (mysql_stmt_bind_result(stmt, result) != 0) {
        LOG_ERROR << "mysql_stmt_bind_result failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return {};
    }
    
    vector<User> vec;
    while (mysql_stmt_fetch(stmt) == 0) {
        User user;
        user.setId(id);
        user.setName(name);
        user.setState(state);
        vec.push_back(user);
    }
    
    mysql_stmt_close(stmt);
    return vec;
}

vector<User> FriendModel::queryFriendRequests(int userid) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return {};
    }
    
    const char* sql = "SELECT a.id, a.name, a.state FROM user a INNER JOIN friend_request b ON b.from_id = a.id WHERE b.to_id = ? AND b.status = 'pending'";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return {};
    }
    
    MYSQL_BIND params[1];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &userid;
    
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return {};
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return {};
    }
    
    MYSQL_BIND result[3];
    memset(result, 0, sizeof(result));
    
    int id = 0;
    char name[50] = {0};
    char state[20] = {0};
    
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &id;
    
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = name;
    result[1].buffer_length = sizeof(name);
    
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = state;
    result[2].buffer_length = sizeof(state);
    
    if (mysql_stmt_bind_result(stmt, result) != 0) {
        LOG_ERROR << "mysql_stmt_bind_result failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return {};
    }
    
    vector<User> vec;
    while (mysql_stmt_fetch(stmt) == 0) {
        User user;
        user.setId(id);
        user.setName(name);
        user.setState(state);
        vec.push_back(user);
    }
    
    mysql_stmt_close(stmt);
    return vec;
}

bool FriendModel::remove(int userid, int friendid) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return false;
    }
    
    const char* sql = "DELETE FROM friend WHERE userid = ? AND friendid = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return false;
    }
    
    MYSQL_BIND params[2];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &userid;
    
    params[1].buffer_type = MYSQL_TYPE_LONG;
    params[1].buffer = &friendid;
    
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
    LOG_INFO << "Friend relationship removed: " << userid << " <-> " << friendid;
    return true;
}

bool FriendModel::sendFriendRequest(int from_id, int to_id) {
    if (from_id == to_id) {
        LOG_WARN << "Cannot send friend request to self";
        return false;
    }
    
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return false;
    }
    
    const char* sql = "INSERT INTO friend_request(from_id, to_id, status) VALUES(?, ?, 'pending')";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return false;
    }
    
    MYSQL_BIND params[2];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &from_id;
    
    params[1].buffer_type = MYSQL_TYPE_LONG;
    params[1].buffer = &to_id;
    
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
    LOG_INFO << "Friend request sent: " << from_id << " -> " << to_id;
    return true;
}

bool FriendModel::handleFriendRequest(int request_id, bool accept) {
    int from_id, to_id;
    string status;
    
    if (!getFriendRequest(request_id, from_id, to_id, status)) {
        return false;
    }
    
    if (status != "pending") {
        LOG_WARN << "Friend request " << request_id << " is not pending";
        return false;
    }
    
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return false;
    }
    
    mysql_autocommit(conn, false);
    
    if (accept) {
        const char* insert_sql = "INSERT INTO friend(userid, friendid) VALUES(?, ?), (?, ?)";
        MYSQL_STMT* insert_stmt = mysql_stmt_init(conn);
        
        if (!insert_stmt) {
            LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
            mysql_rollback(conn);
            return false;
        }
        
        int ids[4] = {from_id, to_id, to_id, from_id};
        MYSQL_BIND insert_params[4];
        memset(insert_params, 0, sizeof(insert_params));
        
        for (int i = 0; i < 4; ++i) {
            insert_params[i].buffer_type = MYSQL_TYPE_LONG;
            insert_params[i].buffer = &ids[i];
        }
        
        if (mysql_stmt_bind_param(insert_stmt, insert_params) != 0) {
            LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(insert_stmt);
            mysql_stmt_close(insert_stmt);
            mysql_rollback(conn);
            return false;
        }
        
        if (mysql_stmt_execute(insert_stmt) != 0) {
            LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(insert_stmt);
            mysql_stmt_close(insert_stmt);
            mysql_rollback(conn);
            return false;
        }
        
        mysql_stmt_close(insert_stmt);
    }
    
    const char* update_sql = "UPDATE friend_request SET status = ? WHERE id = ?";
    MYSQL_STMT* update_stmt = mysql_stmt_init(conn);
    
    if (!update_stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        mysql_rollback(conn);
        return false;
    }
    
    string new_status = accept ? "accepted" : "rejected";
    
    MYSQL_BIND update_params[2];
    memset(update_params, 0, sizeof(update_params));
    
    update_params[0].buffer_type = MYSQL_TYPE_STRING;
    update_params[0].buffer = const_cast<char*>(new_status.c_str());
    update_params[0].buffer_length = new_status.length();
    
    update_params[1].buffer_type = MYSQL_TYPE_LONG;
    update_params[1].buffer = &request_id;
    
    if (mysql_stmt_bind_param(update_stmt, update_params) != 0) {
        LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(update_stmt);
        mysql_stmt_close(update_stmt);
        mysql_rollback(conn);
        return false;
    }
    
    if (mysql_stmt_execute(update_stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(update_stmt);
        mysql_stmt_close(update_stmt);
        mysql_rollback(conn);
        return false;
    }
    
    mysql_stmt_close(update_stmt);
    mysql_commit(conn);
    mysql_autocommit(conn, true);
    
    LOG_INFO << "Friend request " << request_id << " " << (accept ? "accepted" : "rejected");
    return true;
}

bool FriendModel::getFriendRequest(int request_id, int& from_id, int& to_id, string& status) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return false;
    }
    
    const char* sql = "SELECT from_id, to_id, status FROM friend_request WHERE id = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return false;
    }
    
    MYSQL_BIND params[1];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &request_id;
    
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
    
    MYSQL_BIND result[3];
    memset(result, 0, sizeof(result));
    
    char status_buf[20] = {0};
    
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &from_id;
    
    result[1].buffer_type = MYSQL_TYPE_LONG;
    result[1].buffer = &to_id;
    
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = status_buf;
    result[2].buffer_length = sizeof(status_buf);
    
    if (mysql_stmt_bind_result(stmt, result) != 0) {
        LOG_ERROR << "mysql_stmt_bind_result failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_fetch(stmt) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    status = status_buf;
    mysql_stmt_close(stmt);
    return true;
}
