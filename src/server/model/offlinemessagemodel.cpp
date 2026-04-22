#include "offlinemessagemodel.hpp"
#include "db/mysql_pool.h"
#include "util/logging.h"
using namespace std;
using namespace chat::db;

void OfflineMsgModel::insert(int userid, string msg) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return;
    }
    
    const char* sql = "INSERT INTO offlinemessage(userid, message, is_read) VALUES(?, ?, 0)";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return;
    }
    
    MYSQL_BIND params[2];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &userid;
    
    params[1].buffer_type = MYSQL_TYPE_STRING;
    params[1].buffer = const_cast<char*>(msg.c_str());
    params[1].buffer_length = msg.length();
    
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt);
    }
    
    mysql_stmt_close(stmt);
}

void OfflineMsgModel::remove(int userid) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return;
    }
    
    const char* sql = "DELETE FROM offlinemessage WHERE userid = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return;
    }
    
    MYSQL_BIND params[1];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &userid;
    
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt);
    }
    
    mysql_stmt_close(stmt);
}

vector<string> OfflineMsgModel::query(int userid) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return {};
    }
    
    const char* sql = "SELECT message FROM offlinemessage WHERE userid = ?";
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
    
    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));
    
    char message[4096] = {0};
    result[0].buffer_type = MYSQL_TYPE_STRING;
    result[0].buffer = message;
    result[0].buffer_length = sizeof(message);
    
    if (mysql_stmt_bind_result(stmt, result) != 0) {
        LOG_ERROR << "mysql_stmt_bind_result failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return {};
    }
    
    vector<string> vec;
    while (mysql_stmt_fetch(stmt) == 0) {
        vec.push_back(message);
    }
    
    mysql_stmt_close(stmt);
    return vec;
}

vector<OfflineMsg> OfflineMsgModel::queryBatch(int userid, int page, int pageSize) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return {};
    }
    
    int offset = (page - 1) * pageSize;
    const char* sql = "SELECT id, userid, message, create_time, is_read FROM offlinemessage WHERE userid = ? ORDER BY create_time DESC LIMIT ?, ?";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return {};
    }
    
    MYSQL_BIND params[3];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &userid;
    
    params[1].buffer_type = MYSQL_TYPE_LONG;
    params[1].buffer = &offset;
    
    params[2].buffer_type = MYSQL_TYPE_LONG;
    params[2].buffer = &pageSize;
    
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
    
    MYSQL_BIND result[5];
    memset(result, 0, sizeof(result));
    
    int id = 0;
    int uid = 0;
    char message[4096] = {0};
    char create_time[50] = {0};
    int is_read = 0;
    
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &id;
    
    result[1].buffer_type = MYSQL_TYPE_LONG;
    result[1].buffer = &uid;
    
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = message;
    result[2].buffer_length = sizeof(message);
    
    result[3].buffer_type = MYSQL_TYPE_STRING;
    result[3].buffer = create_time;
    result[3].buffer_length = sizeof(create_time);
    
    result[4].buffer_type = MYSQL_TYPE_TINY;
    result[4].buffer = &is_read;
    
    if (mysql_stmt_bind_result(stmt, result) != 0) {
        LOG_ERROR << "mysql_stmt_bind_result failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return {};
    }
    
    vector<OfflineMsg> vec;
    while (mysql_stmt_fetch(stmt) == 0) {
        OfflineMsg msg;
        msg.id = id;
        msg.userid = uid;
        msg.msg = message;
        msg.create_time = create_time;
        msg.is_read = (is_read == 1);
        vec.push_back(msg);
    }
    
    mysql_stmt_close(stmt);
    return vec;
}

int OfflineMsgModel::getCount(int userid) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return 0;
    }
    
    const char* sql = "SELECT COUNT(*) FROM offlinemessage WHERE userid = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return 0;
    }
    
    MYSQL_BIND params[1];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &userid;
    
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return 0;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return 0;
    }
    
    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));
    
    int count = 0;
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &count;
    
    if (mysql_stmt_bind_result(stmt, result) != 0) {
        LOG_ERROR << "mysql_stmt_bind_result failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return 0;
    }
    
    mysql_stmt_fetch(stmt);
    mysql_stmt_close(stmt);
    
    return count;
}

void OfflineMsgModel::markAsRead(int msgId) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return;
    }
    
    const char* sql = "UPDATE offlinemessage SET is_read = 1 WHERE id = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return;
    }
    
    MYSQL_BIND params[1];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &msgId;
    
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt);
    }
    
    mysql_stmt_close(stmt);
}

void OfflineMsgModel::markBatchAsRead(int userid, const vector<int>& msgIds) {
    if (msgIds.empty()) {
        return;
    }
    
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return;
    }
    
    string sql = "UPDATE offlinemessage SET is_read = 1 WHERE userid = ? AND id IN (";
    for (size_t i = 0; i < msgIds.size(); ++i) {
        if (i > 0) {
            sql += ",";
        }
        sql += "?";
    }
    sql += ")";
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return;
    }
    
    std::vector<MYSQL_BIND> params(msgIds.size() + 1);
    memset(params.data(), 0, params.size() * sizeof(MYSQL_BIND));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &userid;
    
    for (size_t i = 0; i < msgIds.size(); ++i) {
        params[i + 1].buffer_type = MYSQL_TYPE_LONG;
        params[i + 1].buffer = const_cast<int*>(&msgIds[i]);
    }
    
    if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
        LOG_ERROR << "mysql_stmt_prepare failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return;
    }
    
    if (mysql_stmt_bind_param(stmt, params.data()) != 0) {
        LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt);
    }
    
    mysql_stmt_close(stmt);
}

void OfflineMsgModel::cleanExpired(int days) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return;
    }
    
    const char* sql = "DELETE FROM offlinemessage WHERE create_time < DATE_SUB(NOW(), INTERVAL ? DAY)";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return;
    }
    
    MYSQL_BIND params[1];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &days;
    
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt);
    }
    
    mysql_stmt_close(stmt);
    LOG_INFO << "OfflineMsgModel::cleanExpired - cleaned messages older than " << days << " days";
}

int OfflineMsgModel::getUnreadCount(int userid) {
    MySQLPool& pool = MySQLPool::instance();
    MySQLConnectionGuard conn_guard(pool);
    MYSQL* conn = conn_guard.get();
    
    if (!conn) {
        LOG_ERROR << "Failed to get MySQL connection";
        return 0;
    }
    
    const char* sql = "SELECT COUNT(*) FROM offlinemessage WHERE userid = ? AND is_read = 0";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        LOG_ERROR << "mysql_stmt_init failed: " << mysql_error(conn);
        return 0;
    }
    
    MYSQL_BIND params[1];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = &userid;
    
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        LOG_ERROR << "mysql_stmt_bind_param failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return 0;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR << "mysql_stmt_execute failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return 0;
    }
    
    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));
    
    int count = 0;
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &count;
    
    if (mysql_stmt_bind_result(stmt, result) != 0) {
        LOG_ERROR << "mysql_stmt_bind_result failed: " << mysql_stmt_error(stmt);
        mysql_stmt_close(stmt);
        return 0;
    }
    
    mysql_stmt_fetch(stmt);
    mysql_stmt_close(stmt);
    
    return count;
}
