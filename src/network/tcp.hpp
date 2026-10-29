#pragma once

#include "network/message.hpp"

#include <winsock2.h>

#include <cstddef>
#include <cstdint>

class TcpConnection {
public:
    explicit TcpConnection(SOCKET socket);
    ~TcpConnection();

    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

    TcpConnection(TcpConnection&& other) noexcept;
    TcpConnection& operator=(TcpConnection&& other) noexcept;

    void send(const Message& message);
    Message receive();

private:
    SOCKET socket_;

    void send_all(
        const uint8_t* data,
        size_t size
    );

    void receive_all(
        uint8_t* data,
        size_t size
    );
};