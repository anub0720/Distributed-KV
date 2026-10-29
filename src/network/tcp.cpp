#include "network/tcp.hpp"

#include "network/frame.hpp"

#include <stdexcept>
#include <vector>

namespace {

uint32_t read_u32(const uint8_t* data) {
    return
        (static_cast<uint32_t>(data[0]) << 24) |
        (static_cast<uint32_t>(data[1]) << 16) |
        (static_cast<uint32_t>(data[2]) << 8) |
        static_cast<uint32_t>(data[3]);
}

}

TcpConnection::TcpConnection(SOCKET socket)
    : socket_(socket) {
}

TcpConnection::~TcpConnection() {
    if (socket_ != INVALID_SOCKET) {
        closesocket(socket_);
    }
}

TcpConnection::TcpConnection(TcpConnection&& other) noexcept
    : socket_(other.socket_) {
    other.socket_ = INVALID_SOCKET;
}

TcpConnection& TcpConnection::operator=(
    TcpConnection&& other
) noexcept {
    if (this != &other) {
        if (socket_ != INVALID_SOCKET) {
            closesocket(socket_);
        }

        socket_ = other.socket_;
        other.socket_ = INVALID_SOCKET;
    }

    return *this;
}

void TcpConnection::send_all(
    const uint8_t* data,
    size_t size
) {
    size_t sent = 0;

    while (sent < size) {
        int result = ::send(
            socket_,
            reinterpret_cast<const char*>(data + sent),
            static_cast<int>(size - sent),
            0
        );

        if (result == SOCKET_ERROR) {
            throw std::runtime_error("TCP send failed");
        }

        sent += static_cast<size_t>(result);
    }
}

void TcpConnection::receive_all(
    uint8_t* data,
    size_t size
) {
    size_t received = 0;

    while (received < size) {
        int result = ::recv(
            socket_,
            reinterpret_cast<char*>(data + received),
            static_cast<int>(size - received),
            0
        );

        if (result == 0) {
            throw std::runtime_error("connection closed");
        }

        if (result == SOCKET_ERROR) {
            throw std::runtime_error("TCP receive failed");
        }

        received += static_cast<size_t>(result);
    }
}

void TcpConnection::send(const Message& message) {
    auto payload = serialize_message(message);
    auto frame = encode_frame(payload);

    send_all(frame.data(), frame.size());
}

Message TcpConnection::receive() {
    uint8_t header[4];

    receive_all(header, 4);

    uint32_t length = read_u32(header);

    std::vector<uint8_t> payload(length);

    receive_all(
        payload.data(),
        payload.size()
    );

    return deserialize_message(payload);
}