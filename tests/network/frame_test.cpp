#include "network/frame.hpp"

#include <gtest/gtest.h>

TEST(FrameTest, RoundTripsPayload) {
    std::vector<uint8_t> payload{
        1, 2, 3, 4, 5
    };

    auto frame = encode_frame(payload);

    auto decoded = decode_frame(frame);

    EXPECT_EQ(decoded, payload);
}

TEST(FrameTest, RejectsIncompleteHeader) {
    std::vector<uint8_t> frame{
        0, 0, 0
    };

    EXPECT_THROW(
        decode_frame(frame),
        std::runtime_error
    );
}

TEST(FrameTest, RejectsIncompletePayload) {
    std::vector<uint8_t> frame{
        0, 0, 0, 5,
        1, 2
    };

    EXPECT_THROW(
        decode_frame(frame),
        std::runtime_error
    );
}