#pragma once

#include "network/tcp.hpp"

#include <winsock2.h>

#include <cstdint>

class TcpServer {
public:
    explicit TcpServer(uint16_t port);
    ~TcpServer();

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    TcpConnection accept();

private:
    SOCKET socket_;
};