#ifndef RECORD_HPP
#define RECORD_HPP

#include <iostream>
#include <string>
#include <chrono>
#include <cstdint>

// Her nesnede olusmasi gereken header
#pragma pack(push, 1)
struct RecordHeader {
    uint32_t crc{0};
    uint64_t timestamp{0};
    uint32_t key_size{0};
    uint32_t value_size{0};
    uint8_t tombstone{0};
};
#pragma pack(pop)

struct Record
{
    RecordHeader header;
    std::string key;
    std::string value;

    // Yeni kayit olusturma
    static Record create(const std::string& k, const std::string& v) {
        Record r;
        r.key = k;
        r.value = v;
        r.header.tombstone = 0;

        // Zaman
        auto now = std::chrono::system_clock::now().time_since_epoch();
        r.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

        r.header.key_size = static_cast<uint32_t>(k.size());
        r.header.value_size = static_cast<uint32_t>(v.size());
        r.header.tombstone = 0;
        return r;
    }

    // Silme
    static Record createTombstone(const std::string& k) {
        Record r = create(k, "");
        r.header.tombstone = 1;
        r.header.value_size = 0;
        return r;
    }

    size_t totalSize() const {
        return sizeof(RecordHeader) + header.key_size + header.value_size;
    }
};

#endif