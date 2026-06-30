#pragma once
#include "runtime/runtime_object.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

class Class;

class Heap {
public:
    Heap() = default;

    RuntimeObject* new_instance(Class& type);
    RuntimeObject* new_primitive_array(ElementType element_type, int32_t length);
    RuntimeObject* new_instance_array(Class& element_type, int32_t length);

    RuntimeObject* class_object_for(Class& mirrored);

    Heap(const Heap&) = delete;
    Heap& operator=(const Heap&) = delete;
private:
    std::vector<std::unique_ptr<RuntimeObject>> objects_;
    std::unordered_map<Class*, RuntimeObject*> class_objects_;
};
