#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <string>
#include <vector>
using namespace std;

class Redis
{
public:
    Redis();
    ~Redis();

    bool connect(const std::string& host = "127.0.0.1", int port = 6379);
    
    bool set(const string& key, const string& value);
    string get(const string& key);
    bool del(const string& key);
    bool push(const string& key, const string& value);
    std::vector<std::string> getList(const string& key);
    bool delList(const string& key);

private:
    redisContext *_context;
};

#endif
