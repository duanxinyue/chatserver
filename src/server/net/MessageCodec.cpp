#include "MessageCodec.h"
#include "logging.h"
#include <arpa/inet.h>

namespace chat {
namespace net {

/**
 * @brief 从缓冲区解码消息（字节流形式）
 * 
 * 采用固定包头协议解决TCP粘包拆包问题：
 * 1. 先读取4字节包头，获取消息长度
 * 2. 根据长度读取完整的消息体
 * 
 * @param buf 输入缓冲区
 * @param message 输出的消息内容
 * @return true 解码成功，false 需要更多数据或出错
 */
bool MessageCodec::decode(muduo::net::Buffer* buf, std::string& message) {
    // 检查是否有足够的数据读取包头
    if (buf->readableBytes() < kHeaderLen) {
        return false;
    }
    
    // 读取包头（4字节，网络字节序）
    const void* header = buf->peek();
    uint32_t message_len = *static_cast<const uint32_t*>(header);
    message_len = ntohl(message_len);  // 转换为主机字节序
    
    // 检查消息长度是否超过最大限制
    if (message_len > kMaxMessageLen) {
        LOG_ERROR << "Message too long: " << message_len;
        return false;
    }
    
    // 检查是否有足够的数据读取消息体
    if (buf->readableBytes() < kHeaderLen + message_len) {
        return false;
    }
    
    // 读取消息体
    buf->retrieve(kHeaderLen);  // 跳过包头
    message.assign(buf->peek(), message_len);
    buf->retrieve(message_len);
    
    LOG_DEBUG << "Decoded message length: " << message_len;
    return true;
}

/**
 * @brief 将消息编码写入缓冲区
 * 
 * 编码格式：4字节长度（网络字节序）+ 消息内容
 * 
 * @param message 消息内容
 * @param buf 输出缓冲区
 */
void MessageCodec::encode(const std::string& message, muduo::net::Buffer* buf) {
    uint32_t message_len = static_cast<uint32_t>(message.size());
    uint32_t network_len = htonl(message_len);  // 转换为网络字节序
    
    buf->append(reinterpret_cast<const char*>(&network_len), kHeaderLen);
    buf->append(message);
    
    LOG_DEBUG << "Encoded message length: " << message_len;
}

/**
 * @brief 将Protobuf消息对象序列化并编码
 * 
 * @param msg Protobuf消息对象
 * @param buf 输出缓冲区
 * @return true 成功，false 失败
 */
bool MessageCodec::encodeMessage(const ChatMessage& msg, muduo::net::Buffer* buf) {
    std::string serialized_msg;
    if (!msg.SerializeToString(&serialized_msg)) {
        LOG_ERROR << "Failed to serialize ChatMessage";
        return false;
    }
    
    encode(serialized_msg, buf);
    return true;
}

/**
 * @brief 解码消息并反序列化为Protobuf对象
 * 
 * @param buf 输入缓冲区
 * @param msg 输出的Protobuf消息对象
 * @return true 成功，false 失败
 */
bool MessageCodec::decodeMessage(muduo::net::Buffer* buf, ChatMessage& msg) {
    std::string serialized_msg;
    if (!decode(buf, serialized_msg)) {
        return false;
    }
    
    if (!msg.ParseFromString(serialized_msg)) {
        LOG_ERROR << "Failed to parse ChatMessage";
        return false;
    }
    
    LOG_DEBUG << "Decoded message type: " << msg.msg_type();
    return true;
}

} // namespace net
} // namespace chat
