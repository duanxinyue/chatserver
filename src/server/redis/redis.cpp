#include "redis.hpp"
#include "logging.h"
#include <vector>
using namespace std;

Redis::Redis() : _context(nullptr) {}

Redis::~Redis() {
    if (_context) {
        redisFree(_context);
    }
}

bool Redis::connect(const std::string& host, int port) {
    _context = redisConnect(host.c_str(), port);
    if (_context == nullptr || _context->err) {
        if (_context) {
            LOG_ERROR << "Redis connect error: " << _context->errstr;
            redisFree(_context);
            _context = nullptr;
        } else {
            LOG_ERROR << "Redis connect error: can't allocate redis context";
        }
        return false;
    }
    LOG_INFO << "Redis connected successfully to " << host << ":" << port;
    return true;
}

bool Redis::set(const string& key, const string& value) {
    if (!_context) return false;
    redisReply* reply = (redisReply*)redisCommand(_context, "SET %s %s", key.c_str(), value.c_str());
    if (!reply) return false;
    freeReplyObject(reply);
    return true;
}

string Redis::get(const string& key) {
    if (!_context) return "";
    redisReply* reply = (redisReply*)redisCommand(_context, "GET %s", key.c_str());
    if (!reply) return "";
    string result = reply->str ? reply->str : "";
    freeReplyObject(reply);
    return result;
}

bool Redis::del(const string& key) {
    if (!_context) return false;
    redisReply* reply = (redisReply*)redisCommand(_context, "DEL %s", key.c_str());
    if (!reply) return false;
    freeReplyObject(reply);
    return true;
}

bool Redis::push(const string& key, const string& value) {
    if (!_context) return false;
    redisReply* reply = (redisReply*)redisCommand(_context, "LPUSH %s %s", key.c_str(), value.c_str());
    if (!reply) return false;
    freeReplyObject(reply);
    return true;
}

std::vector<std::string> Redis::getList(const string& key) {
    std::vector<std::string> result;
    if (!_context) return result;
    redisReply* reply = (redisReply*)redisCommand(_context, "LRANGE %s 0 -1", key.c_str());
    if (!reply || reply->type != REDIS_REPLY_ARRAY) {
        if (reply) freeReplyObject(reply);
        return result;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
        if (reply->element[i]->str) {
            result.push_back(reply->element[i]->str);
        }
    }
    freeReplyObject(reply);
    return result;
}

bool Redis::delList(const string& key) {
    if (!_context) return false;
    redisReply* reply = (redisReply*)redisCommand(_context, "DEL %s", key.c_str());
    if (!reply) return false;
    freeReplyObject(reply);
    return true;
}
