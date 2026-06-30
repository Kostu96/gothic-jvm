#include "runtime/runtime_object.hpp"

#include <cstddef>
#include <stdexcept>

namespace {

Value default_element(ElementType type) {
    switch (type) {
    using enum ElementType;
    case Long:   return static_cast<int64_t>(0);
    case Float:  return 0.0f;
    case Double: return 0.0;
    default:     return static_cast<int32_t>(0);
    }
}

}

PrimitiveArrayData::PrimitiveArrayData(ElementType element_type, int32_t length) :
    element_type(element_type),
    elements(static_cast<size_t>(length), default_element(element_type))
{}

Value PrimitiveArrayData::get(int32_t index) const {
    if (index < 0 || index >= length()) {
        throw std::out_of_range("PrimitiveArrayData: index out of bounds");
    }

    return elements[static_cast<size_t>(index)];
}

void PrimitiveArrayData::set(int32_t index, Value value) {
    if (index < 0 || index >= length()) {
        throw std::out_of_range("PrimitiveArrayData: index out of bounds");
    }

    elements[static_cast<size_t>(index)] = value;
}

InstanceArrayData::InstanceArrayData(Class& element_type, int32_t length) :
    element_type(&element_type),
    elements(static_cast<size_t>(length), nullptr)
{
}

RuntimeObject* InstanceArrayData::get(int32_t index) const {
    if (index < 0 || index >= length()) {
        throw std::out_of_range("InstanceArrayData: index out of bounds");
    }

    return elements[static_cast<size_t>(index)];
}

void InstanceArrayData::set(int32_t index, RuntimeObject* value) {
    if (index < 0 || index >= length()) {
        throw std::out_of_range("InstanceArrayData: index out of bounds");
    }

    elements[static_cast<size_t>(index)] = value;
}
