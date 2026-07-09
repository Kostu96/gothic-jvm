#pragma once
#include "runtime/object.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

class Class;

class Heap {
public:
    Heap() = default;

    Object* new_instance(Class& type);
    Object* new_primitive_array(PrimitiveArrayData::ElementType element_type, int32_t length);
    Object* new_instance_array(Class& element_type, int32_t length);

    Object* class_object_for(Class& mirrored);

    Heap(const Heap&) = delete;
    Heap& operator=(const Heap&) = delete;
private:
    std::vector<std::unique_ptr<Object>> objects_;
    std::unordered_map<Class*, Object*> class_objects_;
};
