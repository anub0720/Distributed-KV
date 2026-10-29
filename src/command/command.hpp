#pragma once
#include <cstdint>
#include <string>

enum class CommandType : uint8_t {
    Set = 1,
    Delete = 2
};

struct Command {
    CommandType type;
    std::string key;
    std::string value;

    static Command set(std::string key, std::string value);
    static Command remove(std::string key);
};