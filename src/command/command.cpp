#include "command/command.hpp"

Command Command::set(std::string key, std::string value) {
    return {
        CommandType::Set,
        std::move(key),
        std::move(value)
    };
}

Command Command::remove(std::string key) {
    return {
        CommandType::Delete,
        std::move(key),
        {}
    };
}