#pragma once

#include "network/node_info.hpp"
#include "raft/log_entry.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct RaftPersistentState {
    uint64_t current_term = 0;
    NodeId voted_for = 0;
    uint64_t commit_index = 0;
    uint64_t last_applied = 0;
    std::vector<LogEntry> entries;
};

class RaftStorage {
public:
    explicit RaftStorage(std::string path);

    std::optional<RaftPersistentState> load();
    void save(const RaftPersistentState& state);

private:
    std::string path_;
};
