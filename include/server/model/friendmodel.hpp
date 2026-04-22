#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include "user.hpp"
#include <vector>
#include <string>

// 维护好友信息的操作接口方法
class FriendModel
{
public:
    // 添加好友关系
    bool insert(int userid, int friendid);

    // 返回用户好友列表
    std::vector<User> query(int userid);
    
    // 获取好友请求列表（查询谁向我发送了好友请求）
    std::vector<User> queryFriendRequests(int userid);
    
    // 删除好友关系
    bool remove(int userid, int friendid);
    
    // 发送好友申请
    bool sendFriendRequest(int from_id, int to_id);
    
    // 处理好友申请（同意/拒绝）
    bool handleFriendRequest(int request_id, bool accept);
    
    // 获取好友申请详情
    bool getFriendRequest(int request_id, int& from_id, int& to_id, std::string& status);
};

#endif
