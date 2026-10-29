#pragma once

#include "network/node_info.hpp"
#include "network/tcp.hpp"
#include "network/transport.hpp"

#include <unordered_map>
#include <mutex>

class TcpTransport : public Transport {
public:
    explicit TcpTransport(
        std::unordered_map<NodeId, NodeInfo> nodes
    );

    void send(
        NodeId destination,
        const Message& message
    ) override;

private:
    std::unordered_map<NodeId, NodeInfo> nodes_;
};