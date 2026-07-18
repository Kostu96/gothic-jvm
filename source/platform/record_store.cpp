#include "platform/record_store.hpp"

#include <stdexcept>

int32_t RecordStore::add_record(std::span<const uint8_t> data) {
    records.push_back(Record{
        .data = std::vector<uint8_t>(data.begin(), data.end())
    });

    return static_cast<int32_t>(records.size());
}

void RecordStore::set_record(int32_t record_id, std::span<const uint8_t> data) {
    if (record_id <= 0 || record_id > static_cast<int32_t>(records.size())) {
        throw std::runtime_error("RecordStore::set_record: invalid record_id");
    }

    records[record_id - 1].data = std::vector<uint8_t>(data.begin(), data.end());
}
