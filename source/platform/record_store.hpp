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

    size_t add_record(std::span<const uint8_t> data);

    size_t size() const noexcept { return records.size(); }
private:
    std::string name;
    std::vector<Record> records;
};
