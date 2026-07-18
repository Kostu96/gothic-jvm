#include "platform/record_store_manager.hpp"

#include <stdexcept>

RecordStore* RecordStoreManager::open_record_store(const std::string& name, bool create_if_necessary) {
    auto it = records.find(name);
    if (it != records.end()) {
        return &it->second;
    }
    else if (create_if_necessary) {
        auto [new_it, _] = records.emplace(name, RecordStore(name));
        return &new_it->second;
    }

    return nullptr;
}
