#include "raft/raft_storage.hpp"

#include "command/command.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

constexpr char MAGIC[] = {'R', 'F', 'S', '1'};
constexpr uint32_t MAX_RECORD_SIZE = 64u * 1024u * 1024u;

void put_u8(std::string& out, uint8_t value) {
    out.push_back(static_cast<char>(value));
}

void put_u32(std::string& out, uint32_t value) {
    for (int shift = 24; shift >= 0; shift -= 8) {
        out.push_back(static_cast<char>((value >> shift) & 0xFF));
    }
}

void put_u64(std::string& out, uint64_t value) {
    for (int shift = 56; shift >= 0; shift -= 8) {
        out.push_back(static_cast<char>((value >> shift) & 0xFF));
    }
}

uint8_t get_u8(const std::string& in, size_t& pos) {
    if (pos >= in.size()) {
        throw std::runtime_error("Truncated Raft state");
    }

    return static_cast<uint8_t>(
        static_cast<unsigned char>(in[pos++])
    );
}

uint32_t get_u32(const std::string& in, size_t& pos) {
    if (in.size() - pos < 4) {
        throw std::runtime_error("Truncated Raft state");
    }

    uint32_t value = 0;

    for (int i = 0; i < 4; ++i) {
        value = (value << 8) |
            static_cast<unsigned char>(in[pos++]);
    }

    return value;
}

uint64_t get_u64(const std::string& in, size_t& pos) {
    if (in.size() - pos < 8) {
        throw std::runtime_error("Truncated Raft state");
    }

    uint64_t value = 0;

    for (int i = 0; i < 8; ++i) {
        value = (value << 8) |
            static_cast<unsigned char>(in[pos++]);
    }

    return value;
}

void put_string(std::string& out, const std::string& value) {
    if (value.size() > std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("Raft string is too large");
    }

    put_u32(out, static_cast<uint32_t>(value.size()));
    out.append(value);
}

std::string get_string(const std::string& in, size_t& pos) {
    const uint32_t length = get_u32(in, pos);

    if (length > in.size() - pos) {
        throw std::runtime_error("Truncated Raft string");
    }

    std::string result = in.substr(pos, length);
    pos += length;
    return result;
}

// FNV-1a checksum for accidental corruption detection.
// This is not a cryptographic integrity check.
uint32_t checksum(const std::string& data) {
    uint32_t hash = 2166136261u;

    for (unsigned char byte : data) {
        hash ^= byte;
        hash *= 16777619u;
    }

    return hash;
}

std::string encode_state(const RaftPersistentState& state) {
    if (state.entries.size() > MAX_RECORD_SIZE / 17u) {
        throw std::runtime_error("Raft log is too large to checkpoint");
    }

    if (state.commit_index > state.entries.size() ||
        state.last_applied > state.commit_index) {
        throw std::runtime_error("Invalid Raft commit/apply indexes");
    }

    std::string out;

    put_u64(out, state.current_term);
    put_u32(out, state.voted_for);
    put_u64(out, state.commit_index);
    put_u64(out, state.last_applied);
    put_u32(out, static_cast<uint32_t>(state.entries.size()));

    uint64_t expected_index = 1;

    for (const LogEntry& entry : state.entries) {
        if (entry.index != expected_index++) {
            throw std::runtime_error("Raft log indexes are not contiguous");
        }

        put_u64(out, entry.index);
        put_u64(out, entry.term);
        put_u8(out, static_cast<uint8_t>(entry.command.type));
        put_string(out, entry.command.key);
        put_string(out, entry.command.value);

        if (out.size() > MAX_RECORD_SIZE) {
            throw std::runtime_error("Raft checkpoint exceeds size limit");
        }
    }

    return out;
}

RaftPersistentState decode_state(const std::string& payload) {
    size_t pos = 0;
    RaftPersistentState state;

    state.current_term = get_u64(payload, pos);
    state.voted_for = get_u32(payload, pos);
    state.commit_index = get_u64(payload, pos);
    state.last_applied = get_u64(payload, pos);

    const uint32_t count = get_u32(payload, pos);

    if (count > payload.size() / 17u) {
        throw std::runtime_error("Invalid Raft log entry count");
    }

    state.entries.reserve(count);

    for (uint32_t i = 0; i < count; ++i) {
        LogEntry entry;

        entry.index = get_u64(payload, pos);
        entry.term = get_u64(payload, pos);

        if (entry.index != static_cast<uint64_t>(i) + 1) {
            throw std::runtime_error("Invalid Raft log index");
        }

        const uint8_t type = get_u8(payload, pos);

        if (type == static_cast<uint8_t>(CommandType::Set)) {
            entry.command.type = CommandType::Set;
        } else if (type == static_cast<uint8_t>(CommandType::Delete)) {
            entry.command.type = CommandType::Delete;
        } else {
            throw std::runtime_error("Invalid command type in Raft log");
        }

        entry.command.key = get_string(payload, pos);
        entry.command.value = get_string(payload, pos);

        state.entries.push_back(std::move(entry));
    }

    if (pos != payload.size()) {
        throw std::runtime_error("Extra bytes in Raft checkpoint");
    }

    if (state.commit_index > state.entries.size() ||
        state.last_applied > state.commit_index) {
        throw std::runtime_error("Invalid Raft commit/apply indexes");
    }

    for (const LogEntry& entry : state.entries) {
        if (entry.term > state.current_term) {
            throw std::runtime_error("Log entry term exceeds current term");
        }
    }

    return state;
}

std::string make_record(const std::string& payload) {
    if (payload.size() > MAX_RECORD_SIZE) {
        throw std::runtime_error("Raft checkpoint exceeds size limit");
    }

    std::string record(MAGIC, sizeof(MAGIC));
    put_u32(record, static_cast<uint32_t>(payload.size()));
    record.append(payload);
    put_u32(record, checksum(payload));

    return record;
}

} // namespace

RaftStorage::RaftStorage(std::string path)
    : path_(std::move(path)) {}

std::optional<RaftPersistentState> RaftStorage::load() {
    if (!std::filesystem::exists(path_)) {
        return std::nullopt;
    }

    std::ifstream file(path_, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Cannot open Raft state file: " + path_);
    }

    const std::string bytes(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    std::optional<RaftPersistentState> latest;
    size_t pos = 0;
    size_t valid_end = 0;

    while (pos < bytes.size()) {
        const size_t record_start = pos;

        // A short or invalid trailing record is treated as a torn write.
        if (bytes.size() - pos < 8) {
            break;
        }

        bool magic_ok = true;

        for (size_t i = 0; i < sizeof(MAGIC); ++i) {
            if (bytes[pos + i] != MAGIC[i]) {
                magic_ok = false;
                break;
            }
        }

        if (!magic_ok) {
            break;
        }

        pos += sizeof(MAGIC);

        uint32_t length = 0;

        try {
            length = get_u32(bytes, pos);
        } catch (...) {
            pos = record_start;
            break;
        }

        if (length > MAX_RECORD_SIZE ||
            bytes.size() - pos < static_cast<size_t>(length) + 4) {
            pos = record_start;
            break;
        }

        const std::string payload = bytes.substr(pos, length);
        pos += length;

        uint32_t stored_checksum = 0;

        try {
            stored_checksum = get_u32(bytes, pos);
        } catch (...) {
            pos = record_start;
            break;
        }

        if (checksum(payload) != stored_checksum) {
            pos = record_start;
            break;
        }

        try {
            latest = decode_state(payload);
        } catch (...) {
            pos = record_start;
            break;
        }

        valid_end = pos;
    }

    if (valid_end != bytes.size()) {
        // Discard any incomplete or corrupt trailing record.
        std::filesystem::resize_file(path_, valid_end);
    }

    return latest;
}

void RaftStorage::save(const RaftPersistentState& state) {
    const std::string payload = encode_state(state);
    const std::string record = make_record(payload);

    std::ofstream file(
        path_,
        std::ios::binary | std::ios::app
    );

    if (!file) {
        throw std::runtime_error("Cannot open Raft state file: " + path_);
    }

    file.write(record.data(), static_cast<std::streamsize>(record.size()));
    file.flush();

    if (!file) {
        throw std::runtime_error("Failed to write Raft state file: " + path_);
    }
}
