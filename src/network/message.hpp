#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class MessageType : uint8_t {
    RequestVote = 1,
    RequestVoteResponse = 2,
    AppendEntries = 3,
    AppendEntriesResponse = 4
};

struct Message {
    MessageType type;

    uint32_t sender;
    uint32_t receiver;

    std::string payload;
};

std::vector<uint8_t> serialize_message(const Message& message);

Message deserialize_message(const std::vector<uint8_t>& data);