
#include "kv/store.hpp"
#include "storage/wal.hpp"

#include <gtest/gtest.h>
#include <filesystem>

class KVStoreTest : public ::testing::Test {
protected:
    std::filesystem::path path =
        "distributed_kv_store_test.wal";

    void SetUp() override {
        std::filesystem::remove(path);
    }

    void TearDown() override {
        std::filesystem::remove(path);
    }
};

TEST_F(KVStoreTest, SetAndGet) {
    WAL wal(path);
    KVStore store(wal);

    EXPECT_TRUE(store.set("name", "alice"));

    auto value = store.get("name");

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "alice");
}

TEST_F(KVStoreTest, MissingKeyReturnsNothing) {
    WAL wal(path);
    KVStore store(wal);

    EXPECT_FALSE(store.get("missing").has_value());
}

TEST_F(KVStoreTest, DeleteRemovesKey) {
    WAL wal(path);
    KVStore store(wal);

    store.set("name", "alice");

    EXPECT_TRUE(store.remove("name"));
    EXPECT_FALSE(store.get("name").has_value());
}

TEST_F(KVStoreTest, DeletingMissingKeyReturnsFalse) {
    WAL wal(path);
    KVStore store(wal);

    EXPECT_FALSE(store.remove("missing"));
}

TEST_F(KVStoreTest, SetOverwritesExistingValue) {
    WAL wal(path);
    KVStore store(wal);

    store.set("name", "alice");
    store.set("name", "bob");

    auto value = store.get("name");

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "bob");
}

TEST_F(KVStoreTest, RecoversAfterRestart) {
    {
        WAL wal(path);
        KVStore store(wal);

        EXPECT_TRUE(store.set("name", "alice"));
        EXPECT_TRUE(store.set("language", "cpp"));
    }

    // Simulated process restart.
    {
        WAL wal(path);
        KVStore store(wal);

        store.recover();

        auto name = store.get("name");
        auto language = store.get("language");

        ASSERT_TRUE(name.has_value());
        ASSERT_TRUE(language.has_value());

        EXPECT_EQ(*name, "alice");
        EXPECT_EQ(*language, "cpp");
    }
}

TEST_F(KVStoreTest, RecoversDeleteAfterRestart) {
    {
        WAL wal(path);
        KVStore store(wal);

        EXPECT_TRUE(store.set("name", "alice"));
        EXPECT_TRUE(store.remove("name"));
    }

    {
        WAL wal(path);
        KVStore store(wal);

        store.recover();

        EXPECT_FALSE(store.get("name").has_value());
    }
}
