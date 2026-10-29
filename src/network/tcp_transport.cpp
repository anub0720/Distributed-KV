#include "network/tcp_transport.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdexcept>

TcpTransport::TcpTransport(
    std::unordered_map<NodeId, NodeInfo> nodes
)
    : nodes_(std::move(nodes)) {
}

void TcpTransport::send(
    NodeId destination,
    const Message& message
) {
    auto it = nodes_.find(destination);

    if (it == nodes_.end()) {
        throw std::runtime_error(
            "unknown destination node"
        );
    }

    const NodeInfo& node = it->second;

    SOCKET socket = ::socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (socket == INVALID_SOCKET) {
        throw std::runtime_error(
            "failed to create TCP socket"
        );
    }

    sockaddr_in address{};

    address.sin_family = AF_INET;
    address.sin_port = htons(node.port);

    if (inet_pton(
            AF_INET,
            node.host.c_str(),
            &address.sin_addr
        ) != 1) {

        closesocket(socket);

        throw std::runtime_error(
            "invalid node address"
        );
    }

    if (::connect(
            socket,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)
        ) == SOCKET_ERROR) {

        closesocket(socket);

        throw std::runtime_error(
            "failed to connect to node"
        );
    }

    TcpConnection connection(socket);

    connection.send(message);
}