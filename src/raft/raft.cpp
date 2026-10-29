#include "raft/raft.hpp"

#include "network/message.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>
#include <iostream>
#include <string>

using Clock = std::chrono::steady_clock;

Raft::Raft(
    NodeId id,
    const std::unordered_map<NodeId, NodeInfo>& nodes,
    Transport& transport,
    KVStore& store
)
    : id_(id),
      role_(RaftRole::Follower),
      current_term_(0),
      voted_for_(0),
      votes_received_(0),
      commit_index_(0),
      last_applied_(0),
      nodes_(nodes),
      transport_(transport),
      store_(store),
      storage_("node_" + std::to_string(id) + ".raft"),
      rng_(std::random_device{}() ^ (id * 7919u)),
      election_deadline_(Clock::now()),
      last_heartbeat_(Clock::now()) {
    if (nodes_.find(id_) == nodes_.end()) {
        throw std::invalid_argument("Raft node ID is not in the cluster");
    }

    if (auto saved = storage_.load()) {
        current_term_ = saved->current_term;
        voted_for_ = saved->voted_for;
        commit_index_ = saved->commit_index;
        last_applied_ = saved->last_applied;

        for (const LogEntry& entry : saved->entries) {
            log_.append(entry);
        }

        // A crash may have happened after commitment was saved
        // but before every committed command was applied.
        apply_committed_locked();
    }

    reset_election_deadline_locked();
    persist_locked();
}

NodeId Raft::id() const {
    return id_;
}

RaftRole Raft::role() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return role_;
}

uint64_t Raft::current_term() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_term_;
}

void Raft::reset_election_deadline_locked() {
    // Election timeouts: 350–550 ms, randomized for each reset.
    std::uniform_int_distribution<int> delay(350, 550);
    election_deadline_ = Clock::now()
        + std::chrono::milliseconds(delay(rng_));
}

void Raft::become_follower_locked(uint64_t new_term) {
    if (new_term > current_term_) {
        current_term_ = new_term;
        voted_for_ = 0;
        persist_locked();
    }

    role_ = RaftRole::Follower;
    votes_received_ = 0;
    votes_from_.clear();
    reset_election_deadline_locked();
}

void Raft::tick() {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto now = Clock::now();

    if (role_ == RaftRole::Leader) {
        if (now - last_heartbeat_ >= std::chrono::milliseconds(100)) {
            last_heartbeat_ = now;

            for (const auto& [peer_id, info] : nodes_) {
                (void)info;

                if (peer_id != id_) {
                    send_append_entries_locked(peer_id);
                }
            }
        }

        return;
    }

    if (now >= election_deadline_) {
        start_election_locked();
    }
}

void Raft::start_election_locked() {
    ++current_term_;
    role_ = RaftRole::Candidate;
    voted_for_ = id_;
    persist_locked();

    votes_received_ = 1;
    votes_from_.clear();
    votes_from_.insert(id_);

    reset_election_deadline_locked();

    RequestVote request{
        current_term_,
        id_,
        log_.last_index(),
        log_.last_term()
    };

    for (const auto& [peer_id, info] : nodes_) {
        (void)info;

        if (peer_id == id_) {
            continue;
        }

        Message message{
            MessageType::RequestVote,
            id_,
            peer_id,
            serialize_request_vote(request)
        };

        try {
            transport_.send(peer_id, message);
        }
        catch (const std::exception& e) {
            // A peer can be down during an election.
            // The candidate can still win with a reachable majority.
            (void)e;
        }
    }

    // This also makes the implementation work for a one-node cluster.
    if (votes_received_ > nodes_.size() / 2) {
        become_leader_locked();
    }
}

RequestVoteResponse Raft::handle_request_vote(
    const RequestVote& request
) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (request.term < current_term_) {
        return RequestVoteResponse{current_term_, false};
    }

    if (request.term > current_term_) {
        become_follower_locked(request.term);
    }

    const bool candidate_log_is_up_to_date =
        request.last_log_term > log_.last_term() ||
        (request.last_log_term == log_.last_term() &&
         request.last_log_index >= log_.last_index());

    const bool can_vote =
        (voted_for_ == 0 || voted_for_ == request.candidate_id);

    const bool grant = can_vote && candidate_log_is_up_to_date;

    if (grant) {
        voted_for_ = request.candidate_id;
        role_ = RaftRole::Follower;
        reset_election_deadline_locked();
        persist_locked();
    }

    return RequestVoteResponse{current_term_, grant};
}

void Raft::handle_request_vote_response(
    NodeId sender,
    const RequestVoteResponse& response
) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (response.term > current_term_) {
        become_follower_locked(response.term);
        return;
    }

    if (role_ != RaftRole::Candidate ||
        response.term != current_term_ ||
        !response.vote_granted) {
        return;
    }

    // Count each peer at most once per election.
    if (!votes_from_.insert(sender).second) {
        return;
    }

    ++votes_received_;

    if (votes_received_ > nodes_.size() / 2) {
        become_leader_locked();
    }
}

void Raft::become_leader_locked() {
    role_ = RaftRole::Leader;
    std::cout << "\n[LEADER] Node " << id_
          << " elected for term " << current_term_
          << " with a majority of votes\n";
    votes_received_ = 0;

    const uint64_t next = log_.last_index() + 1;

    next_index_.clear();
    match_index_.clear();

    for (const auto& [peer_id, info] : nodes_) {
        (void)info;

        if (peer_id == id_) {
            continue;
        }

        next_index_[peer_id] = next;
        match_index_[peer_id] = 0;
    }

    // The leader counts its own log when determining a majority.
    match_index_[id_] = log_.last_index();

    last_heartbeat_ = Clock::now();

    // Send an immediate heartbeat rather than waiting for the timer.
    for (const auto& [peer_id, info] : nodes_) {
        (void)info;

        if (peer_id != id_) {
            send_append_entries_locked(peer_id);
        }
    }
}

void Raft::send_append_entries_locked(NodeId peer) {
    if (role_ != RaftRole::Leader) {
        return;
    }

    uint64_t next = next_index_[peer];

    if (next == 0) {
        next = 1;
    }

    if (next > log_.last_index() + 1) {
        next = log_.last_index() + 1;
    }

    next_index_[peer] = next;

    const uint64_t previous_index = next - 1;

    AppendEntries request{
        current_term_,
        id_,
        previous_index,
        log_.term_at(previous_index),
        log_.entries_from(next),
        commit_index_
    };

    Message message{
        MessageType::AppendEntries,
        id_,
        peer,
        serialize_append_entries(request)
    };

    try {
        transport_.send(peer, message);
    }
    catch (const std::exception& e) {
        (void)e;
    }
}

void Raft::send_heartbeats() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (role_ != RaftRole::Leader) {
        return;
    }

    last_heartbeat_ = Clock::now();

    for (const auto& [peer_id, info] : nodes_) {
        (void)info;

        if (peer_id != id_) {
            send_append_entries_locked(peer_id);
        }
    }
}

AppendEntriesResponse Raft::handle_append_entries(
    const AppendEntries& request
) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (request.term < current_term_) {
        return AppendEntriesResponse{
            current_term_, false, log_.last_index()
        };
    }

    if (request.term > current_term_) {
        become_follower_locked(request.term);
    } else if (role_ != RaftRole::Follower) {
        role_ = RaftRole::Follower;
        votes_received_ = 0;
        votes_from_.clear();
    }

    // A valid-term leader contact postpones another election.
    reset_election_deadline_locked();

    if (request.prev_log_index > log_.last_index()) {
        return AppendEntriesResponse{
            current_term_, false, log_.last_index()
        };
    }

    if (log_.term_at(request.prev_log_index) != request.prev_log_term) {
        return AppendEntriesResponse{
            current_term_, false, log_.last_index()
        };
    }

    for (const LogEntry& entry : request.entries) {
        if (entry.index <= log_.last_index()) {
            if (log_.at(entry.index).term != entry.term) {
                // Never overwrite an entry that is already committed.
                if (entry.index <= commit_index_) {
                    persist_locked();
                    return AppendEntriesResponse{
                        current_term_, false, log_.last_index()
                    };
                }

                log_.truncate_from(entry.index);
                log_.append(entry);
            }
            // If terms match, Raft assumes the existing entry matches too.
        } else if (entry.index == log_.last_index() + 1) {
            log_.append(entry);
        } else {
            persist_locked();
            return AppendEntriesResponse{
                current_term_, false, log_.last_index()
            };
        }
    }

    persist_locked();

    if (request.leader_commit > commit_index_) {
        commit_index_ = std::min(
            request.leader_commit,
            log_.last_index()
        );

        persist_locked();
        apply_committed_locked();
    }

    const uint64_t matched_index = request.entries.empty()
        ? request.prev_log_index
        : request.entries.back().index;

    return AppendEntriesResponse{
        current_term_, true, matched_index
    };
}

void Raft::handle_append_entries_response(
    NodeId sender,
    const AppendEntriesResponse& response
) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (response.term > current_term_) {
        become_follower_locked(response.term);
        return;
    }

    if (role_ != RaftRole::Leader ||
        response.term != current_term_) {
        return;
    }

    if (response.success) {
        if (response.match_index > log_.last_index()) {
            return;
        }

        match_index_[sender] = std::max(
            match_index_[sender],
            response.match_index
        );

        next_index_[sender] = match_index_[sender] + 1;

        advance_commit_index_locked();
    } else {
        uint64_t& next = next_index_[sender];

        if (next > 1) {
            --next;
        }

        // Retry with an earlier prefix; eventually this can send
        // the follower the complete log starting at index 1.
        send_append_entries_locked(sender);
    }
}

bool Raft::propose(const Command& command) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (role_ != RaftRole::Leader) {
        return false;
    }

    const uint64_t index = log_.last_index() + 1;

    log_.append(LogEntry{
        index,
        current_term_,
        command
    });
    persist_locked();

    match_index_[id_] = index;

    for (const auto& [peer_id, info] : nodes_) {
        (void)info;

        if (peer_id != id_) {
            send_append_entries_locked(peer_id);
        }
    }

    advance_commit_index_locked();
    return true;
}

std::optional<std::string> Raft::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return store_.get(key);
}

void Raft::advance_commit_index_locked() {
    for (uint64_t index = log_.last_index();
         index > commit_index_;
         --index) {
        // A leader may only advance commitIndex directly using an
        // entry from its current term.
        if (log_.term_at(index) != current_term_) {
            continue;
        }

        size_t replicated = 0;

        for (const auto& [peer_id, info] : nodes_) {
            (void)info;

            if (peer_id == id_) {
                if (log_.last_index() >= index) {
                    ++replicated;
                }
            } else if (match_index_[peer_id] >= index) {
                ++replicated;
            }
        }

        if (replicated > nodes_.size() / 2) {
            commit_index_ = index;
            persist_locked();
            apply_committed_locked();
            break;
        }
    }
}

void Raft::apply_committed_locked() {
    while (last_applied_ < commit_index_) {
        const uint64_t index = last_applied_ + 1;
        const LogEntry& entry = log_.at(index);

        if (entry.command.type == CommandType::Set) {
            store_.set(entry.command.key, entry.command.value);
        } else if (entry.command.type == CommandType::Delete) {
            store_.remove(entry.command.key);
        } else {
            throw std::runtime_error("Unknown command in committed Raft log");
        }

        last_applied_ = index;
        persist_locked();
    }
}

void Raft::persist_locked() {
    RaftPersistentState state;

    state.current_term = current_term_;
    state.voted_for = voted_for_;
    state.commit_index = commit_index_;
    state.last_applied = last_applied_;
    state.entries = log_.entries_from(1);

    storage_.save(state);
}