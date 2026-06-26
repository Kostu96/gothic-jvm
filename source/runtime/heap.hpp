#pragma once
#include "runtime/object.hpp"

#include <cstdint>
#include <memory>
#include <vector>

class Class;

class Heap {
public:
    Heap() = default;

    ArrayObject* new_array(ArrayType element_type, int32_t length);
    InstanceObject* new_instance(Class* type);

    Heap(const Heap&) = delete;
    Heap& operator=(const Heap&) = delete;
private:
    std::vector<std::unique_ptr<Object>> objects_;
};
