#include "network/message.hpp"
#include <gtest/gtest.h>

TEST(MessageTest, RoundTripsRequestVote) {
    Message original{
        MessageType::RequestVote,
        1,
        2,
        "term=1 candidate=1"
    };

    auto encoded = serialize_message(original);
    auto decoded = deserialize_message(encoded);

    EXPECT_EQ(decoded.type, MessageType::RequestVote);
    EXPECT_EQ(decoded.sender, 1);
    EXPECT_EQ(decoded.receiver, 2);
    EXPECT_EQ(decoded.payload, "term=1 candidate=1");
}