#pragma once

#include <cstdint>
#include <string>

using NodeId = uint32_t;

struct NodeInfo {
    NodeId id;
    std::string host;
    uint16_t port;
};