#pragma once

#include "command/command.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

class WAL {
public:
    explicit WAL(const std::filesystem::path& path);
    ~WAL();

    WAL(const WAL&) = delete;
    WAL& operator=(const WAL&) = delete;

    void append(const Command& command);
    void sync();

    std::vector<Command> recover();

private:
    std::filesystem::path path_;
    std::ofstream file_;

    void write_u32(uint32_t value);
    static uint32_t read_u32(const std::vector<uint8_t>& data, size_t& offset);

    static std::vector<uint8_t> serialize(const Command& command);
    static Command deserialize(
        CommandType type,
        const std::vector<uint8_t>& payload
    );

    static uint32_t crc32(const std::vector<uint8_t>& data);
};