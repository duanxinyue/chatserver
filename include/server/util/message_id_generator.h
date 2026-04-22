#ifndef MESSAGE_ID_GENERATOR_H
#define MESSAGE_ID_GENERATOR_H

#include <atomic>

namespace chat {
namespace util {

class MessageIdGenerator {
public:
    static MessageIdGenerator& instance() {
        static MessageIdGenerator generator;
        return generator;
    }

    uint64_t generate() {
        return _counter.fetch_add(1);
    }

private:
    MessageIdGenerator() : _counter(1) {}
    std::atomic<uint64_t> _counter;
};

} // namespace util
} // namespace chat

#endif // MESSAGE_ID_GENERATOR_H
