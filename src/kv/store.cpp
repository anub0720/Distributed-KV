#include "kv/store.hpp"

KVStore::KVStore(WAL& wal)
    : wal_(wal) {
}

bool KVStore::set(const std::string& key, const std::string& value) {
    Command command = Command::set(key, value);

    wal_.append(command);
    wal_.sync();

    apply(command);

    return true;
}

std::optional<std::string> KVStore::get(const std::string& key) const {
    auto it = data_.find(key);

    if (it == data_.end()) {
        return std::nullopt;
    }

    return it->second;
}

bool KVStore::remove(const std::string& key) {
    if (data_.find(key) == data_.end()) {
        return false;
    }

    Command command = Command::remove(key);

    wal_.append(command);
    wal_.sync();

    apply(command);

    return true;
}

void KVStore::recover() {
    auto commands = wal_.recover();

    for (const auto& command : commands) {
        apply(command);
    }
}

void KVStore::apply(const Command& command) {
    switch (command.type) {
        case CommandType::Set:
            data_[command.key] = command.value;
            break;

        case CommandType::Delete:
            data_.erase(command.key);
            break;
    }
}