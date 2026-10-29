#include "network/message.hpp"

#include <stdexcept>
#include <vector>

namespace {

void append_u32(
    std::vector<uint8_t>& data,
    uint32_t value
) {
    data.push_back(static_cast<uint8_t>(value >> 24));
    data.push_back(static_cast<uint8_t>(value >> 16));
    data.push_back(static_cast<uint8_t>(value >> 8));
    data.push_back(static_cast<uint8_t>(value));
}

uint32_t read_u32(
    const std::vector<uint8_t>& data,
    size_t& offset
) {
    if (offset + 4 > data.size()) {
        throw std::runtime_error("unexpected end of message");
    }

    uint32_t value =
        (static_cast<uint32_t>(data[offset]) << 24) |
        (static_cast<uint32_t>(data[offset + 1]) << 16) |
        (static_cast<uint32_t>(data[offset + 2]) << 8) |
        static_cast<uint32_t>(data[offset + 3]);

    offset += 4;

    return value;
}

} // namespace

std::vector<uint8_t> serialize_message(const Message& message) {
    std::vector<uint8_t> data;

    data.push_back(static_cast<uint8_t>(message.type));

    append_u32(data, message.sender);
    append_u32(data, message.receiver);

    append_u32(
        data,
        static_cast<uint32_t>(message.payload.size())
    );

    data.insert(
        data.end(),
        message.payload.begin(),
        message.payload.end()
    );

    return data;
}

Message deserialize_message(
    const std::vector<uint8_t>& data
) {
    size_t offset = 0;

    if (data.empty()) {
        throw std::runtime_error("empty message");
    }

    Message message;

    message.type =
        static_cast<MessageType>(data[offset++]);

    message.sender = read_u32(data, offset);
    message.receiver = read_u32(data, offset);

    uint32_t payload_size =
        read_u32(data, offset);

    if (offset + payload_size != data.size()) {
        throw std::runtime_error("invalid payload size");
    }

    message.payload.assign(
        reinterpret_cast<const char*>(data.data() + offset),
        payload_size
    );

    return message;
}