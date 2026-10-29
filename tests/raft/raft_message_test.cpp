#include "raft/raft_message.hpp"

#include <gtest/gtest.h>

TEST(RaftMessageTest, RoundTripsEmptyAppendEntries) {
    AppendEntries original{
        5,
        1,
        0,
        0,
        {},
        0
    };

    auto payload =
        serialize_append_entries(original);

    auto decoded =
        deserialize_append_entries(payload);

    EXPECT_EQ(decoded.term, 5);
    EXPECT_EQ(decoded.leader_id, 1);
    EXPECT_EQ(decoded.prev_log_index, 0);
    EXPECT_EQ(decoded.prev_log_term, 0);
    EXPECT_TRUE(decoded.entries.empty());
    EXPECT_EQ(decoded.leader_commit, 0);
}

TEST(RaftMessageTest, RoundTripsAppendEntriesWithCommand) {
    AppendEntries original{
        3,
        1,
        2,
        2,
        {
            LogEntry{
                3,
                3,
                Command::set("x", "10")
            }
        },
        2
    };

    auto payload =
        serialize_append_entries(original);

    auto decoded =
        deserialize_append_entries(payload);

    ASSERT_EQ(decoded.entries.size(), 1);

    EXPECT_EQ(decoded.term, 3);
    EXPECT_EQ(decoded.leader_id, 1);
    EXPECT_EQ(decoded.prev_log_index, 2);
    EXPECT_EQ(decoded.prev_log_term, 2);

    EXPECT_EQ(decoded.entries[0].index, 3);
    EXPECT_EQ(decoded.entries[0].term, 3);
    EXPECT_EQ(decoded.entries[0].command.type, CommandType::Set);
    EXPECT_EQ(decoded.entries[0].command.key, "x");
    EXPECT_EQ(decoded.entries[0].command.value, "10");

    EXPECT_EQ(decoded.leader_commit, 2);
}