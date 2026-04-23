#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"
#include <string>

// User表的数据操作类
class UserModel {
public:
    // User表的增加方法
    bool insert(chatserver::User& user);

    // 根据用户号码查询用户信息
    chatserver::User query(int id);

    // 更新用户的状态信息
    bool updateState(chatserver::User user);

    // 重置用户的状态信息
    void resetState();
    
    // 验证密码
    bool verifyPassword(int id, const std::string& password);
    
    // 获取用户状态
    std::string getState(int id);
};

#endif