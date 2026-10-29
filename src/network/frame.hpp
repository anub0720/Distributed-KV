#pragma once

#include <cstdint>
#include <vector>

std::vector<uint8_t> encode_frame(
    const std::vector<uint8_t>& payload
);

std::vector<uint8_t> decode_frame(
    const std::vector<uint8_t>& data
);