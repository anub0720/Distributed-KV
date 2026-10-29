#pragma once

#include "command/command.hpp"

#include <cstdint>

struct LogEntry {
    uint64_t index;
    uint64_t term;
    Command command;
};