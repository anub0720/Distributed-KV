
#include "kv/store.hpp"
#include "storage/wal.hpp"

#include <iostream>

int main() {
    WAL wal("distributed_kv_store.wal");
    KVStore store(wal);

    store.set("name", "alice");

    auto value = store.get("name");

    if (value.has_value()) {
        std::cout << *value << '\n';
    }

    return 0;
}

