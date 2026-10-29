#pragma once

#include "command/command.hpp"
#include "storage/wal.hpp"

#include <optional>
#include <string>
#include <unordered_map>

class KVStore {
public:
    explicit KVStore(WAL& wal);

    bool set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool remove(const std::string& key);

    void recover();

private:
    WAL& wal_;
    std::unordered_map<std::string, std::string> data_;

    void apply(const Command& command);
};