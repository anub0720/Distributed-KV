#pragma once

#include "network/node_info.hpp"
#include "raft/log_entry.hpp"

#include <cstdint>
#include <string>
#include <vector>

struct RequestVote {
    uint64_t term;
    NodeId candidate_id;
    uint64_t last_log_index;
    uint64_t last_log_term;
};

struct RequestVoteResponse {
    uint64_t term;
    bool vote_granted;
};

struct AppendEntries {
    uint64_t term;
    NodeId leader_id;

    uint64_t prev_log_index;
    uint64_t prev_log_term;

    std::vector<LogEntry> entries;

    uint64_t leader_commit;
};

struct AppendEntriesResponse {
    uint64_t term;
    bool success;
    uint64_t match_index;
};

// Raft message encoding.
std::string serialize_append_entries(const AppendEntries& message);
AppendEntries deserialize_append_entries(const std::string& payload);

std::string serialize_append_entries_response(
    const AppendEntriesResponse& message
);

AppendEntriesResponse deserialize_append_entries_response(
    const std::string& payload
);
std::string serialize_request_vote(const RequestVote& request);
RequestVote deserialize_request_vote(const std::string& payload);

std::string serialize_request_vote_response(
    const RequestVoteResponse& response
);
RequestVoteResponse deserialize_request_vote_response(
    const std::string& payload
);