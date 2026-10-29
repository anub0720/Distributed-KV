#include "network/network.hpp"

#include <winsock2.h>

#include <stdexcept>

void initialize_network() {
    WSADATA data{};

    int result = WSAStartup(
        MAKEWORD(2, 2),
        &data
    );

    if (result != 0) {
        throw std::runtime_error(
            "WSAStartup failed"
        );
    }
}

void shutdown_network() {
    WSACleanup();
}