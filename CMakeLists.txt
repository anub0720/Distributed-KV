cmake_minimum_required(VERSION 3.20)

project(distributed_kv
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# ============================================================
# KV NODE
# ============================================================

add_executable(kv_node
    src/main.cpp

    src/kv/store.cpp
    src/command/command.cpp
    src/storage/wal.cpp
)

target_include_directories(kv_node PRIVATE
    src
)


# ============================================================
# TESTS
# ============================================================

enable_testing()

find_package(GTest CONFIG REQUIRED)

add_executable(kv_tests
    tests/kv/store_test.cpp
    tests/storage/wal_test.cpp

    tests/network/message_test.cpp
    tests/network/frame_test.cpp
    tests/raft/raft_message_test.cpp
    src/kv/store.cpp
    src/command/command.cpp
    src/storage/wal.cpp

    src/network/message.cpp
    src/network/frame.cpp
    src/raft/raft_storage.cpp
    src/raft/raft.cpp
    src/raft/log.cpp
    src/raft/raft_message.cpp
)

target_include_directories(kv_tests PRIVATE
    src
)

target_link_libraries(kv_tests PRIVATE
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(kv_tests)


# ============================================================
# TCP NODE
# ============================================================


add_executable(node
    src/network/node.cpp

    src/network/tcp.cpp
    src/network/tcp_server.cpp
    src/network/tcp_transport.cpp
    src/network/network.cpp
    src/command/command.cpp
    src/network/message.cpp
    src/network/frame.cpp
    src/raft/raft_storage.cpp
     src/raft/raft.cpp
     src/raft/log.cpp
     src/raft/raft_message.cpp
     src/kv/store.cpp
src/storage/wal.cpp
)

target_include_directories(node PRIVATE
    src
)

target_link_libraries(node PRIVATE
    ws2_32
)