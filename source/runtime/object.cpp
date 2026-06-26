#include "runtime/object.hpp"

#include <cstddef>
#include <stdexcept>

namespace {

Value default_element(ArrayType type) {
    switch (type) {
    case ArrayType::Long:   return static_cast<int64_t>(0);
    case ArrayType::Float:  return 0.0f;
    case ArrayType::Double: return 0.0;
    default:                return static_cast<int32_t>(0);
    }
}

}

ArrayObject::ArrayObject(ArrayType element_type, int32_t length) :
    Object(Kind::Array),
    element_type_(element_type),
    elements_(static_cast<size_t>(length), default_element(element_type))
{}

Value ArrayObject::get(int32_t index) const {
    if (index < 0 || index >= elements_.size()) {
        throw std::out_of_range("ArrayObject: index out of bounds");
    }

    return elements_[static_cast<size_t>(index)];
}

void ArrayObject::set(int32_t index, Value value) {
    if (index < 0 || index >= elements_.size()) {
        throw std::out_of_range("ArrayObject: index out of bounds");
    }

    elements_[static_cast<size_t>(index)] = value;
}
