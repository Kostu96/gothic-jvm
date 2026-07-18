#include "platform/record_store.hpp"

size_t RecordStore::add_record(std::span<const uint8_t> data) {
    records.push_back(Record{
        .data = std::vector<uint8_t>(data.begin(), data.end())
        });
    return records.size();
}
