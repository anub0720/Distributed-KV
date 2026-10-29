#include "network/tcp_server.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdexcept>

TcpServer::TcpServer(uint16_t port)
    : socket_(INVALID_SOCKET) {

    socket_ = ::socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (socket_ == INVALID_SOCKET) {
        throw std::runtime_error("failed to create socket");
    }

    sockaddr_in address{};

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    if (::bind(
            socket_,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)
        ) == SOCKET_ERROR) {

        closesocket(socket_);
        throw std::runtime_error("bind failed");
    }

    if (::listen(socket_, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(socket_);
        throw std::runtime_error("listen failed");
    }
}

TcpServer::~TcpServer() {
    if (socket_ != INVALID_SOCKET) {
        closesocket(socket_);
    }
}

TcpConnection TcpServer::accept() {
    SOCKET client = ::accept(
        socket_,
        nullptr,
        nullptr
    );

    if (client == INVALID_SOCKET) {
        throw std::runtime_error("accept failed");
    }

    return TcpConnection(client);
}