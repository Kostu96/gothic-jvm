#pragma once
#include "platform/record_store.hpp"

#include <unordered_map>

class RecordStoreManager {
public:
    RecordStoreManager() = default;

    RecordStore* open_record_store(const std::string& name, bool create_if_necessary);
private:
    std::unordered_map<std::string, RecordStore> records;
};
