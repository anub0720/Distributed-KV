#include "network/frame.hpp"

#include <stdexcept>

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
    const std::vector<uint8_t>& data
) {
    if (data.size() < 4) {
        throw std::runtime_error("frame too small");
    }

    return
        (static_cast<uint32_t>(data[0]) << 24) |
        (static_cast<uint32_t>(data[1]) << 16) |
        (static_cast<uint32_t>(data[2]) << 8) |
        static_cast<uint32_t>(data[3]);
}

} // namespace

std::vector<uint8_t> encode_frame(
    const std::vector<uint8_t>& payload
) {
    std::vector<uint8_t> frame;

    append_u32(
        frame,
        static_cast<uint32_t>(payload.size())
    );

    frame.insert(
        frame.end(),
        payload.begin(),
        payload.end()
    );

    return frame;
}

std::vector<uint8_t> decode_frame(
    const std::vector<uint8_t>& data
) {
    if (data.size() < 4) {
        throw std::runtime_error("incomplete frame header");
    }

    uint32_t length = read_u32(data);

    if (data.size() != 4 + length) {
        throw std::runtime_error("incomplete or invalid frame");
    }

    return std::vector<uint8_t>(
        data.begin() + 4,
        data.end()
    );
}