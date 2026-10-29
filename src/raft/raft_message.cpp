#include "raft/raft_message.hpp"

#include <cstring>
#include <stdexcept>
#include <utility>
namespace {

void write_u64(std::string& output, uint64_t value) {
    for (int i = 7; i >= 0; --i) {
        output.push_back(
            static_cast<char>((value >> (i * 8)) & 0xFF)
        );
    }
}

uint64_t read_u64(
    const std::string& input,
    size_t& offset
) {
    if (offset + 8 > input.size()) {
        throw std::runtime_error("invalid Raft message");
    }

    uint64_t value = 0;

    for (int i = 0; i < 8; ++i) {
        value <<= 8;
        value |= static_cast<uint8_t>(input[offset++]);
    }

    return value;
}

void write_u32(std::string& output, uint32_t value) {
    for (int i = 3; i >= 0; --i) {
        output.push_back(
            static_cast<char>((value >> (i * 8)) & 0xFF)
        );
    }
}

uint32_t read_u32(
    const std::string& input,
    size_t& offset
) {
    if (offset + 4 > input.size()) {
        throw std::runtime_error("invalid Raft message");
    }

    uint32_t value = 0;

    for (int i = 0; i < 4; ++i) {
        value <<= 8;
        value |= static_cast<uint8_t>(input[offset++]);
    }

    return value;
}

void write_command(
    std::string& output,
    const Command& command
) {
    output.push_back(
        static_cast<char>(command.type)
    );

    write_u32(
        output,
        static_cast<uint32_t>(command.key.size())
    );

    output.append(command.key);

    write_u32(
        output,
        static_cast<uint32_t>(command.value.size())
    );

    output.append(command.value);
}

Command read_command(
    const std::string& input,
    size_t& offset
) {
    if (offset >= input.size()) {
        throw std::runtime_error("invalid command");
    }

    auto type =
        static_cast<CommandType>(
            static_cast<uint8_t>(input[offset++])
        );

    uint32_t key_size = read_u32(input, offset);

    if (offset + key_size > input.size()) {
        throw std::runtime_error("invalid command key");
    }

    std::string key =
        input.substr(offset, key_size);

    offset += key_size;

    uint32_t value_size = read_u32(input, offset);

    if (offset + value_size > input.size()) {
        throw std::runtime_error("invalid command value");
    }

    std::string value =
        input.substr(offset, value_size);

    offset += value_size;

    if (type == CommandType::Set) {
        return Command::set(
            std::move(key),
            std::move(value)
        );
    }

    if (type == CommandType::Delete) {
        return Command::remove(
            std::move(key)
        );
    }

    throw std::runtime_error("unknown command type");
}

} // namespace

std::string serialize_append_entries(
    const AppendEntries& message
) {
    std::string output;

    write_u64(output, message.term);
    write_u32(output, message.leader_id);

    write_u64(output, message.prev_log_index);
    write_u64(output, message.prev_log_term);

    write_u32(
        output,
        static_cast<uint32_t>(message.entries.size())
    );

    for (const auto& entry : message.entries) {
        write_u64(output, entry.index);
        write_u64(output, entry.term);
        write_command(output, entry.command);
    }

    write_u64(output, message.leader_commit);

    return output;
}

AppendEntries deserialize_append_entries(
    const std::string& payload
) {
    size_t offset = 0;

    AppendEntries message{};

    message.term =
        read_u64(payload, offset);

    message.leader_id =
        read_u32(payload, offset);

    message.prev_log_index =
        read_u64(payload, offset);

    message.prev_log_term =
        read_u64(payload, offset);

    uint32_t entry_count =
        read_u32(payload, offset);

    message.entries.reserve(entry_count);

    for (uint32_t i = 0; i < entry_count; ++i) {
        LogEntry entry{};

        entry.index =
            read_u64(payload, offset);

        entry.term =
            read_u64(payload, offset);

        entry.command =
            read_command(payload, offset);

        message.entries.push_back(
            std::move(entry)
        );
    }

    message.leader_commit =
        read_u64(payload, offset);

    if (offset != payload.size()) {
        throw std::runtime_error(
            "extra bytes in AppendEntries"
        );
    }

    return message;
}

std::string serialize_append_entries_response(
    const AppendEntriesResponse& message
) {
    std::string output;

    write_u64(output, message.term);

    output.push_back(
        static_cast<char>(
            message.success ? 1 : 0
        )
    );

    write_u64(output, message.match_index);

    return output;
}

AppendEntriesResponse deserialize_append_entries_response(
    const std::string& payload
) {
    size_t offset = 0;

    AppendEntriesResponse message{};

    message.term =
        read_u64(payload, offset);

    if (offset >= payload.size()) {
        throw std::runtime_error(
            "invalid AppendEntriesResponse"
        );
    }

    message.success =
        static_cast<uint8_t>(
            payload[offset++]
        ) != 0;

    message.match_index =
        read_u64(payload, offset);

    if (offset != payload.size()) {
        throw std::runtime_error(
            "extra bytes in AppendEntriesResponse"
        );
    }

    return message;
}
namespace {

void vote_write_u64(std::string& out, uint64_t value) {
    for (int shift = 56; shift >= 0; shift -= 8) {
        out.push_back(static_cast<char>((value >> shift) & 0xFF));
    }
}

uint64_t vote_read_u64(const std::string& in, size_t& pos) {
    if (pos + 8 > in.size()) {
        throw std::runtime_error("Incomplete RequestVote payload");
    }

    uint64_t value = 0;

    for (int i = 0; i < 8; ++i) {
        value = (value << 8) |
            static_cast<unsigned char>(in[pos++]);
    }

    return value;
}

void vote_write_u32(std::string& out, uint32_t value) {
    for (int shift = 24; shift >= 0; shift -= 8) {
        out.push_back(static_cast<char>((value >> shift) & 0xFF));
    }
}

uint32_t vote_read_u32(const std::string& in, size_t& pos) {
    if (pos + 4 > in.size()) {
        throw std::runtime_error("Incomplete candidate ID");
    }

    uint32_t value = 0;

    for (int i = 0; i < 4; ++i) {
        value = (value << 8) |
            static_cast<unsigned char>(in[pos++]);
    }

    return value;
}

} // namespace

std::string serialize_request_vote(const RequestVote& request) {
    std::string out;

    vote_write_u64(out, request.term);
    vote_write_u32(out, request.candidate_id);
    vote_write_u64(out, request.last_log_index);
    vote_write_u64(out, request.last_log_term);

    return out;
}

RequestVote deserialize_request_vote(const std::string& payload) {
    size_t pos = 0;

    RequestVote request{
        vote_read_u64(payload, pos),
        vote_read_u32(payload, pos),
        vote_read_u64(payload, pos),
        vote_read_u64(payload, pos)
    };

    if (pos != payload.size()) {
        throw std::runtime_error("Extra bytes in RequestVote payload");
    }

    return request;
}

std::string serialize_request_vote_response(
    const RequestVoteResponse& response
) {
    std::string out;

    vote_write_u64(out, response.term);
    out.push_back(response.vote_granted ? '\1' : '\0');

    return out;
}

RequestVoteResponse deserialize_request_vote_response(
    const std::string& payload
) {
    size_t pos = 0;

    uint64_t term = vote_read_u64(payload, pos);

    if (pos >= payload.size()) {
        throw std::runtime_error("Incomplete RequestVoteResponse payload");
    }

    unsigned char granted =
        static_cast<unsigned char>(payload[pos++]);

    if (granted > 1 || pos != payload.size()) {
        throw std::runtime_error("Invalid RequestVoteResponse payload");
    }

    return RequestVoteResponse{term, granted == 1};
}