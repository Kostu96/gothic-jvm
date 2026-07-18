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

    void set_string_class(Class& string_class) noexcept { string_class_ = &string_class; }

    Object* new_instance(Class& type);
    Object* new_primitive_array(PrimitiveArrayData::ElementType element_type, int32_t length);
    Object* new_instance_array(Class& element_type, int32_t length);

    // TODO(Kostu): fix interning. String should not be interned eagerly.
    Object* new_interned_string(std::string_view str);

    Object* class_object_for(Class& mirrored);

    Heap(const Heap&) = delete;
    Heap& operator=(const Heap&) = delete;
private:
    Class* string_class_ = nullptr;
    std::vector<std::unique_ptr<Object>> objects_;
    std::unordered_map<std::string, Object*> string_objects_;
    std::unordered_map<Class*, Object*> class_objects_;
};
