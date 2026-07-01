#pragma once
#include "runtime/class.hpp"
#include "runtime/value.hpp"

#include <cstdint>
#include <variant>
#include <vector>

class Class;

enum class ElementType : uint8_t {
    Boolean = 4,
    Char = 5,
    Float = 6,
    Double = 7,
    Byte = 8,
    Short = 9,
    Int = 10,
    Long = 11,
};

struct InstanceData {
    Class* type;
    std::vector<Value> fields;
};

struct PrimitiveArrayData {
    PrimitiveArrayData(ElementType element_type, int32_t length);

    ElementType element_type;
    std::vector<Value> elements;

    int32_t length() const noexcept { return static_cast<int32_t>(elements.size()); }
    Value get(int32_t index) const;
    void set(int32_t index, Value value);
};

struct InstanceArrayData {
    InstanceArrayData(Class& element_type, int32_t length);

    Class* element_type; // class of the array's elements
    std::vector<RuntimeObject*> elements;

    int32_t length() const noexcept { return static_cast<int32_t>(elements.size()); }
    RuntimeObject* get(int32_t index) const;
    void set(int32_t index, RuntimeObject* value);
};

struct ClassMirrorData {
    Class* mirrored;
};

struct RuntimeObject {
    std::variant<InstanceData, PrimitiveArrayData, InstanceArrayData, ClassMirrorData> data;
};
