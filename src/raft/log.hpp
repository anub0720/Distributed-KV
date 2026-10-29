
#pragma once

#include "raft/log_entry.hpp"

#include <cstdint>
#include <vector>

class RaftLog {
public:
    uint64_t last_index() const;
    uint64_t last_term() const;
    uint64_t term_at(uint64_t index) const;

    void append(LogEntry entry);
    void truncate_from(uint64_t index);

    const LogEntry& at(uint64_t index) const;
    std::vector<LogEntry> entries_from(uint64_t index) const;

    size_t size() const;

private:
    std::vector<LogEntry> entries_;
};