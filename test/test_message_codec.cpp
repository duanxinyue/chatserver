#include <gtest/gtest.h>
#include "net/MessageCodec.h"
#include "muduo/net/Buffer.h"

using namespace chat::net;
using namespace muduo::net;

TEST(MessageCodecTest, EncodeDecodeNormalMessage) {
    std::string original = "Hello, World!";
    Buffer buf;
    
    MessageCodec::encode(original, &buf);
    
    EXPECT_EQ(buf.readableBytes(), MessageCodec::kHeaderLen + original.size());
    
    std::string decoded;
    EXPECT_TRUE(MessageCodec::decode(&buf, decoded));
    EXPECT_EQ(original, decoded);
    EXPECT_EQ(buf.readableBytes(), 0);
}

TEST(MessageCodecTest, EncodeDecodeEmptyMessage) {
    std::string original = "";
    Buffer buf;
    
    MessageCodec::encode(original, &buf);
    
    EXPECT_EQ(buf.readableBytes(), MessageCodec::kHeaderLen);
    
    std::string decoded;
    EXPECT_TRUE(MessageCodec::decode(&buf, decoded));
    EXPECT_EQ(original, decoded);
}

TEST(MessageCodecTest, EncodeDecodeLargeMessage) {
    std::string original(1024 * 10, 'x');
    Buffer buf;
    
    MessageCodec::encode(original, &buf);
    
    EXPECT_EQ(buf.readableBytes(), MessageCodec::kHeaderLen + original.size());
    
    std::string decoded;
    EXPECT_TRUE(MessageCodec::decode(&buf, decoded));
    EXPECT_EQ(original, decoded);
}

TEST(MessageCodecTest, PartialMessageShouldWait) {
    std::string original = std::string(100, 'x');
    Buffer buf;
    
    MessageCodec::encode(original, &buf);
    size_t total_size = buf.readableBytes();
    
    std::string partial(buf.peek(), total_size - 50);
    buf.retrieve(total_size - 50);
    
    std::string decoded;
    EXPECT_FALSE(MessageCodec::decode(&buf, decoded));
    
    buf.append(partial.data() + (total_size - 50), 50);
    EXPECT_TRUE(MessageCodec::decode(&buf, decoded));
    EXPECT_EQ(original, decoded);
}

TEST(MessageCodecTest, MultipleMessages) {
    std::string msg1 = "First message";
    std::string msg2 = "Second message";
    std::string msg3 = "Third message";
    
    Buffer buf;
    MessageCodec::encode(msg1, &buf);
    MessageCodec::encode(msg2, &buf);
    MessageCodec::encode(msg3, &buf);
    
    std::string decoded1, decoded2, decoded3;
    EXPECT_TRUE(MessageCodec::decode(&buf, decoded1));
    EXPECT_TRUE(MessageCodec::decode(&buf, decoded2));
    EXPECT_TRUE(MessageCodec::decode(&buf, decoded3));
    
    EXPECT_EQ(msg1, decoded1);
    EXPECT_EQ(msg2, decoded2);
    EXPECT_EQ(msg3, decoded3);
}

TEST(MessageCodecTest, MessageTooLargeShouldFail) {
    std::string original(MessageCodec::kMaxMessageLen + 1, 'x');
    Buffer buf;
    
    MessageCodec::encode(original, &buf);
    
    std::string decoded;
    EXPECT_FALSE(MessageCodec::decode(&buf, decoded));
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
