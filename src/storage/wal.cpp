#include "storage/wal.hpp"

#include <array>
#include <cstring>
#include <stdexcept>

namespace {

constexpr uint32_t MAGIC = 0x4B564C31; // "KVL1"

void append_u32(std::vector<uint8_t>& out, uint32_t value) {
    out.push_back(static_cast<uint8_t>(value >> 24));
    out.push_back(static_cast<uint8_t>(value >> 16));
    out.push_back(static_cast<uint8_t>(value >> 8));
    out.push_back(static_cast<uint8_t>(value));
}

uint32_t read_u32_at(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + 4 > data.size()) {
        throw std::runtime_error("unexpected end of record");
    }

    uint32_t value =
        (static_cast<uint32_t>(data[offset]) << 24) |
        (static_cast<uint32_t>(data[offset + 1]) << 16) |
        (static_cast<uint32_t>(data[offset + 2]) << 8) |
        static_cast<uint32_t>(data[offset + 3]);

    offset += 4;
    return value;
}

} // namespace

WAL::WAL(const std::filesystem::path& path)
    : path_(path),
      file_(path_, std::ios::binary | std::ios::app) {

    if (!file_) {
        throw std::runtime_error("failed to open WAL");
    }
}

WAL::~WAL() {
    if (file_.is_open()) {
        file_.flush();
        file_.close();
    }
}

void WAL::append(const Command& command) {
    auto payload = serialize(command);

    uint32_t length =
        static_cast<uint32_t>(1 + payload.size());

    std::vector<uint8_t> record;

    append_u32(record, MAGIC);
    append_u32(record, length);

    record.push_back(static_cast<uint8_t>(command.type));

    record.insert(
        record.end(),
        payload.begin(),
        payload.end()
    );

    uint32_t checksum = crc32(record);

    append_u32(record, checksum);

    file_.write(
        reinterpret_cast<const char*>(record.data()),
        static_cast<std::streamsize>(record.size())
    );

    if (!file_) {
        throw std::runtime_error("failed to write WAL");
    }
}

void WAL::sync() {
    file_.flush();

    if (!file_) {
        throw std::runtime_error("failed to flush WAL");
    }

    // We'll make the OS-level durability operation explicit here
    // when we finalize the Windows-specific storage implementation.
}

std::vector<uint8_t> WAL::serialize(const Command& command) {
    std::vector<uint8_t> payload;

    append_u32(payload, static_cast<uint32_t>(command.key.size()));

    payload.insert(
        payload.end(),
        command.key.begin(),
        command.key.end()
    );

    if (command.type == CommandType::Set) {
        append_u32(payload, static_cast<uint32_t>(command.value.size()));

        payload.insert(
            payload.end(),
            command.value.begin(),
            command.value.end()
        );
    }

    return payload;
}

Command WAL::deserialize(
    CommandType type,
    const std::vector<uint8_t>& payload) {

    size_t offset = 0;

    uint32_t key_size = read_u32_at(payload, offset);

    if (offset + key_size > payload.size()) {
        throw std::runtime_error("invalid key size");
    }

    std::string key(
        payload.begin() + static_cast<std::ptrdiff_t>(offset),
        payload.begin() + static_cast<std::ptrdiff_t>(offset + key_size)
    );

    offset += key_size;

    if (type == CommandType::Delete) {
        if (offset != payload.size()) {
            throw std::runtime_error("invalid DELETE payload");
        }

        return Command::remove(std::move(key));
    }

    if (type == CommandType::Set) {
        uint32_t value_size = read_u32_at(payload, offset);

        if (offset + value_size != payload.size()) {
            throw std::runtime_error("invalid SET payload");
        }

        std::string value(
            payload.begin() + static_cast<std::ptrdiff_t>(offset),
            payload.end()
        );

        return Command::set(
            std::move(key),
            std::move(value)
        );
    }

    throw std::runtime_error("unknown command type");
}

uint32_t WAL::crc32(const std::vector<uint8_t>& data) {
    uint32_t crc = 0xFFFFFFFF;

    for (uint8_t byte : data) {
        crc ^= byte;

        for (int i = 0; i < 8; ++i) {
            crc = (crc >> 1) ^
                  (0xEDB88320 & -(crc & 1));
        }
    }

    return ~crc;
}
std::vector<Command> WAL::recover() {
    std::vector<Command> commands;

    std::ifstream input(path_, std::ios::binary);

    if (!input) {
        // No WAL yet.
        return commands;
    }

    input.seekg(0, std::ios::end);
    const auto file_size = input.tellg();
    input.seekg(0, std::ios::beg);

    std::streamoff last_good_offset = 0;
    bool truncate_tail = false;

    while (input.tellg() < file_size) {
        const auto record_start = input.tellg();

        // ------------------------------------------------------------
        // Read MAGIC
        // ------------------------------------------------------------

        std::array<uint8_t, 4> magic_bytes{};

        input.read(
            reinterpret_cast<char*>(magic_bytes.data()),
            magic_bytes.size()
        );

        if (input.gcount() != static_cast<std::streamsize>(magic_bytes.size())) {
            // Incomplete final record.
            truncate_tail = true;
            break;
        }

        std::vector<uint8_t> header(
            magic_bytes.begin(),
            magic_bytes.end()
        );

        size_t offset = 0;
        uint32_t magic = read_u32_at(header, offset);

        if (magic != MAGIC) {
            throw std::runtime_error("WAL corruption: invalid magic");
        }

        // ------------------------------------------------------------
        // Read LENGTH
        // ------------------------------------------------------------

        std::array<uint8_t, 4> length_bytes{};

        input.read(
            reinterpret_cast<char*>(length_bytes.data()),
            length_bytes.size()
        );

        if (input.gcount() != static_cast<std::streamsize>(length_bytes.size())) {
            // Incomplete final record.
            truncate_tail = true;
            break;
        }

        std::vector<uint8_t> length_data(
            length_bytes.begin(),
            length_bytes.end()
        );

        offset = 0;
        uint32_t length = read_u32_at(length_data, offset);

        // We already know TYPE is 1 byte.
        if (length < 1) {
            throw std::runtime_error("WAL corruption: invalid length");
        }

        // ------------------------------------------------------------
        // Make sure the complete body + checksum exists
        // ------------------------------------------------------------

        const auto current_position = input.tellg();

        const auto remaining =
            file_size - current_position;

        const auto required =
            static_cast<std::streamoff>(length) + 4;

        if (remaining < required) {
            // The record was only partially written.
            truncate_tail = true;
            break;
        }

        // ------------------------------------------------------------
        // Read TYPE + PAYLOAD
        // ------------------------------------------------------------

        std::vector<uint8_t> body(length);

        input.read(
            reinterpret_cast<char*>(body.data()),
            static_cast<std::streamsize>(body.size())
        );

        if (input.gcount() != static_cast<std::streamsize>(body.size())) {
            truncate_tail = true;
            break;
        }

        // ------------------------------------------------------------
        // Read CHECKSUM
        // ------------------------------------------------------------

        std::array<uint8_t, 4> checksum_bytes{};

        input.read(
            reinterpret_cast<char*>(checksum_bytes.data()),
            checksum_bytes.size()
        );

        if (input.gcount() != static_cast<std::streamsize>(checksum_bytes.size())) {
            truncate_tail = true;
            break;
        }

        std::vector<uint8_t> stored_checksum_data(
            checksum_bytes.begin(),
            checksum_bytes.end()
        );

        offset = 0;
        uint32_t stored_checksum =
            read_u32_at(stored_checksum_data, offset);

        // ------------------------------------------------------------
        // Reconstruct record without checksum
        // ------------------------------------------------------------

        std::vector<uint8_t> record;

        append_u32(record, MAGIC);
        append_u32(record, length);

        record.insert(
            record.end(),
            body.begin(),
            body.end()
        );

        uint32_t calculated_checksum = crc32(record);

        if (calculated_checksum != stored_checksum) {
            // We know this record is complete, but corrupt.
            //
            // If this is the final record, treat it as a torn tail.
            // Otherwise, corruption exists in the middle of the WAL.
            const auto after_record = input.tellg();

            if (after_record == file_size) {
                break;
            }

            throw std::runtime_error(
                "WAL corruption: checksum mismatch"
            );
        }

        // ------------------------------------------------------------
        // Deserialize
        // ------------------------------------------------------------

        CommandType type =
            static_cast<CommandType>(body[0]);

        std::vector<uint8_t> payload(
            body.begin() + 1,
            body.end()
        );

        commands.push_back(
            deserialize(type, payload)
        );

        last_good_offset = input.tellg();
    }

    input.close();

    if (truncate_tail) {
        std::filesystem::resize_file(
            path_,
            static_cast<std::uintmax_t>(last_good_offset)
        );
    }

    file_.close();

    file_.open(
        path_,
        std::ios::binary | std::ios::app
    );

    if (!file_) {
        throw std::runtime_error("failed to reopen WAL after recovery");
    }

    return commands;
}
