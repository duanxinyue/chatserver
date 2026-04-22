#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>
using namespace std;

struct OfflineMsg {
    int id;
    int userid;
    string msg;
    string create_time;
    bool is_read;
};

// 提供离线消息表的操作接口方法
class OfflineMsgModel
{
public:
    // 存储用户的离线消息
    void insert(int userid, string msg);

    // 删除用户的离线消息
    void remove(int userid);

    // 查询用户的离线消息
    vector<string> query(int userid);
    
    // 批量拉取离线消息（带分页）
    vector<OfflineMsg> queryBatch(int userid, int page, int pageSize);
    
    // 获取离线消息总数
    int getCount(int userid);
    
    // 标记消息为已读
    void markAsRead(int msgId);
    
    // 批量标记消息为已读
    void markBatchAsRead(int userid, const vector<int>& msgIds);
    
    // 清理过期消息（比如超过7天）
    void cleanExpired(int days);
    
    // 获取未读消息数量
    int getUnreadCount(int userid);
};

#endif