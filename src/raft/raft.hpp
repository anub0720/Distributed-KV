#pragma once

#include "command/command.hpp"
#include "kv/store.hpp"
#include "network/node_info.hpp"
#include "network/transport.hpp"
#include "raft/log.hpp"
#include "raft/raft_message.hpp"
#include "raft/raft_storage.hpp"
#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>

enum class RaftRole {
    Follower,
    Candidate,
    Leader
};

class Raft {
public:
    Raft(
        NodeId id,
        const std::unordered_map<NodeId, NodeInfo>& nodes,
        Transport& transport,
        KVStore& store
    );

    NodeId id() const;
    RaftRole role() const;
    uint64_t current_term() const;

    // Called periodically by the node's timer thread.
    void tick();

    // Election handlers.
    RequestVoteResponse handle_request_vote(
        const RequestVote& request
    );

    void handle_request_vote_response(
        NodeId sender,
        const RequestVoteResponse& response
    );

    // Replication handlers.
    void send_heartbeats();

    AppendEntriesResponse handle_append_entries(
        const AppendEntries& request
    );

    void handle_append_entries_response(
        NodeId sender,
        const AppendEntriesResponse& response
    );

    bool propose(const Command& command);
    std::optional<std::string> get(const std::string& key);

private:
    NodeId id_;
    RaftRole role_;
    uint64_t current_term_;
    NodeId voted_for_;
    uint32_t votes_received_;

    RaftLog log_;
    uint64_t commit_index_;
    uint64_t last_applied_;

    std::unordered_map<NodeId, uint64_t> next_index_;
    std::unordered_map<NodeId, uint64_t> match_index_;
    std::unordered_set<NodeId> votes_from_;

    const std::unordered_map<NodeId, NodeInfo>& nodes_;
    Transport& transport_;
    KVStore& store_;
    RaftStorage storage_;

    mutable std::mutex mutex_;

    std::mt19937 rng_;
    std::chrono::steady_clock::time_point election_deadline_;
    std::chrono::steady_clock::time_point last_heartbeat_;

    void reset_election_deadline_locked();
    void start_election_locked();
    void become_leader_locked();
    void become_follower_locked(uint64_t new_term);

    void send_append_entries_locked(NodeId peer);
    void advance_commit_index_locked();
    void apply_committed_locked();
    void persist_locked();
};
