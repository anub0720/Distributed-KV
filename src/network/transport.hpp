#pragma once

#include "network/message.hpp"
#include "network/node_info.hpp"

class Transport {
public:
    virtual ~Transport() = default;

    virtual void send(
        NodeId destination,
        const Message& message
    ) = 0;
};