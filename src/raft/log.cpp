
#include "raft/log.hpp"

#include <stdexcept>
#include <utility>

uint64_t RaftLog::last_index() const {
    return entries_.empty() ? 0 : entries_.back().index;
}

uint64_t RaftLog::last_term() const {
    return entries_.empty() ? 0 : entries_.back().term;
}

uint64_t RaftLog::term_at(uint64_t index) const {
    if (index == 0) {
        return 0;
    }

    return at(index).term;
}

void RaftLog::append(LogEntry entry) {
    if (entry.index != last_index() + 1) {
        throw std::invalid_argument("Raft log index is not contiguous");
    }

    entries_.push_back(std::move(entry));
}

void RaftLog::truncate_from(uint64_t index) {
    if (index == 0) {
        entries_.clear();
        return;
    }

    if (index <= entries_.size()) {
        entries_.resize(static_cast<size_t>(index - 1));
    }
}

const LogEntry& RaftLog::at(uint64_t index) const {
    if (index == 0 || index > entries_.size()) {
        throw std::out_of_range("Raft log index out of range");
    }

    return entries_[static_cast<size_t>(index - 1)];
}

std::vector<LogEntry> RaftLog::entries_from(uint64_t index) const {
    if (index == 0) {
        index = 1;
    }

    if (index > last_index() + 1) {
        throw std::out_of_range("Raft log start index out of range");
    }

    return std::vector<LogEntry>(
        entries_.begin() + static_cast<std::ptrdiff_t>(index - 1),
        entries_.end()
    );
}

size_t RaftLog::size() const {
    return entries_.size();
}