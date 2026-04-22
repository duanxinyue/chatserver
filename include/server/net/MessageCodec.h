#ifndef MESSAGE_CODEC_H
#define MESSAGE_CODEC_H

#include <muduo/net/Buffer.h>
#include <string>
#include "chat.pb.h"

namespace chat {
namespace net {

/**
 * @brief 消息编解码器
 * 
 * 负责TCP消息的编解码，采用固定包头协议：
 * - 包头：4字节，存储消息长度（网络字节序）
 * - 包体：Protobuf序列化后的消息数据
 * 
 * 解决TCP粘包拆包问题，确保消息完整性
 */
class MessageCodec {
public:
    static const size_t kHeaderLen = 4;          // 固定包头长度（4字节）
    static const size_t kMaxMessageLen = 65536;  // 最大消息长度（64KB）
    
    /**
     * @brief 从缓冲区解码消息
     * @param buf 输入缓冲区
     * @param message 输出的消息内容（Protobuf序列化数据）
     * @return true 解码成功，false 需要更多数据
     */
    static bool decode(muduo::net::Buffer* buf, std::string& message);
    
    /**
     * @brief 将消息编码写入缓冲区
     * @param message 消息内容（Protobuf序列化数据）
     * @param buf 输出缓冲区
     */
    static void encode(const std::string& message, muduo::net::Buffer* buf);
    
    /**
     * @brief 获取包头长度
     * @return 包头长度
     */
    static size_t getHeaderLength() { return kHeaderLen; }
    
    /**
     * @brief 将Protobuf消息对象序列化并编码
     * @param msg Protobuf消息对象
     * @param buf 输出缓冲区
     * @return true 成功，false 失败
     */
    static bool encodeMessage(const ChatMessage& msg, muduo::net::Buffer* buf);
    
    /**
     * @brief 解码消息并反序列化为Protobuf对象
     * @param buf 输入缓冲区
     * @param msg 输出的Protobuf消息对象
     * @return true 成功，false 失败
     */
    static bool decodeMessage(muduo::net::Buffer* buf, ChatMessage& msg);
};

} // namespace net
} // namespace chat

#endif // MESSAGE_CODEC_H
