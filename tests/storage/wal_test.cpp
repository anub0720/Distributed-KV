#include "storage/wal.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

class WALTest : public ::testing::Test {
protected:
    std::filesystem::path path =
        std::filesystem::temp_directory_path() /
        "distributed_kv_test.wal";

    void SetUp() override {
        std::filesystem::remove(path);
    }

    void TearDown() override {
        std::filesystem::remove(path);
    }
};

TEST_F(WALTest, RecoversSingleSet) {
    {
        WAL wal(path);

        wal.append(Command::set("name", "alice"));
        wal.sync();
    }

    WAL wal(path);

    auto commands = wal.recover();

    ASSERT_EQ(commands.size(), 1);
    EXPECT_EQ(commands[0].type, CommandType::Set);
    EXPECT_EQ(commands[0].key, "name");
    EXPECT_EQ(commands[0].value, "alice");
}

TEST_F(WALTest, RecoversMultipleCommands) {
    {
        WAL wal(path);

        wal.append(Command::set("name", "alice"));
        wal.append(Command::set("language", "cpp"));
        wal.append(Command::remove("name"));

        wal.sync();
    }

    WAL wal(path);

    auto commands = wal.recover();

    ASSERT_EQ(commands.size(), 3);

    EXPECT_EQ(commands[0].type, CommandType::Set);
    EXPECT_EQ(commands[1].type, CommandType::Set);
    EXPECT_EQ(commands[2].type, CommandType::Delete);
}

TEST_F(WALTest, EmptyWalRecoversNothing) {
    WAL wal(path);

    auto commands = wal.recover();

    EXPECT_TRUE(commands.empty());
}
TEST_F(WALTest, IgnoresIncompleteFinalRecord) {
    {
        WAL wal(path);

        wal.append(Command::set("x", "10"));
        wal.append(Command::set("y", "20"));
        wal.append(Command::set("z", "30"));

        wal.sync();
    }

    auto file_size = std::filesystem::file_size(path);

    // Remove the last 3 bytes from the final record.
    std::filesystem::resize_file(path, file_size - 3);

    WAL wal(path);

    auto commands = wal.recover();

    ASSERT_EQ(commands.size(), 2);

    EXPECT_EQ(commands[0].key, "x");
    EXPECT_EQ(commands[0].value, "10");

    EXPECT_EQ(commands[1].key, "y");
    EXPECT_EQ(commands[1].value, "20");
}
TEST_F(WALTest, DetectsCorruptionInMiddle) {
    {
        WAL wal(path);

        wal.append(Command::set("x", "10"));
        wal.append(Command::set("y", "20"));
        wal.append(Command::set("z", "30"));

        wal.sync();
    }

    // Read the whole WAL.
    std::ifstream input(path, std::ios::binary);

    ASSERT_TRUE(input);

    std::vector<uint8_t> data(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>()
    );

    ASSERT_GT(data.size(), 20);

    /*
     * Corrupt a byte somewhere inside the second record.
     *
     * We don't need to know the exact record structure here.
     * The first record is small, so offset 20 is safely beyond it
     * and inside the second record for our current test data.
     */
    data[20] ^= 0xFF;

    input.close();

    std::ofstream output(path, std::ios::binary | std::ios::trunc);

    ASSERT_TRUE(output);

    output.write(
        reinterpret_cast<const char*>(data.data()),
        static_cast<std::streamsize>(data.size())
    );

    output.close();

    WAL wal(path);

    EXPECT_THROW(
        wal.recover(),
        std::runtime_error
    );
}