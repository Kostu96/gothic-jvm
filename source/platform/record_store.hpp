#pragma once
#include <span>
#include <string>
#include <vector>

class RecordStore {
public:
    struct Record {
        std::vector<uint8_t> data;
    };

    explicit RecordStore(const std::string& name) : name(name) {}

    int32_t add_record(std::span<const uint8_t> data);
    void set_record(int32_t record_id, std::span<const uint8_t> data);

    int32_t size() const noexcept { return static_cast<int32_t>(records.size()); }
private:
    std::string name;
    std::vector<Record> records;
};
