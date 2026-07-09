#include "runtime/object.hpp"

#include <cstddef>
#include <stdexcept>

namespace {

Value default_element(PrimitiveArrayData::ElementType type) {
    switch (type) {
    using enum PrimitiveArrayData::ElementType;
    case Long:   return static_cast<int64_t>(0);
    case Float:  return 0.0f;
    case Double: return 0.0;
    default:     return static_cast<int32_t>(0);
    }
}

}

PrimitiveArrayData::PrimitiveArrayData(ElementType type, int32_t length) {
    switch (type) {
        using enum ElementType;
    case Boolean:
    case Byte:   elements = std::vector<uint8_t>(static_cast<size_t>(length), 0); break;
    case Char:   elements = std::vector<char16_t>(static_cast<size_t>(length), 0); break;
    case Short:  elements = std::vector<int16_t>(static_cast<size_t>(length), 0); break;
    case Int:    elements = std::vector<int32_t>(static_cast<size_t>(length), 0); break;
    case Long:   elements = std::vector<int64_t>(static_cast<size_t>(length), 0); break;
    case Float:  elements = std::vector<float>(static_cast<size_t>(length), 0.0f); break;
    case Double: elements = std::vector<double>(static_cast<size_t>(length), 0.0); break;
    }
}

int32_t PrimitiveArrayData::length() const noexcept {
    return static_cast<int32_t>(std::visit([](auto&& vec) { return vec.size(); }, elements));
}

Value PrimitiveArrayData::get(int32_t index) const {
    if (index < 0 || index >= length()) {
        throw std::out_of_range("ArrayData: index out of bounds");
    }

    return std::visit([index](auto&& vec) -> Value {
        using T = std::decay_t<decltype(vec)>;
        if constexpr (std::is_same_v<T, std::vector<uint8_t>> || 
                      std::is_same_v<T, std::vector<char16_t>> ||
                      std::is_same_v<T, std::vector<int16_t>> ||
                      std::is_same_v<T, std::vector<int32_t>>) {
            return static_cast<int32_t>(vec[static_cast<size_t>(index)]);
        }
        else {
            return vec[static_cast<size_t>(index)];
        }
    }, elements);
}

void PrimitiveArrayData::set(int32_t index, Value value) {
    if (index < 0 || index >= length()) {
        throw std::out_of_range("ArrayData: index out of bounds");
    }

    std::visit([index, value](auto&& vec) {
        using T = std::decay_t<decltype(vec)>;
        if constexpr (std::is_same_v<T, std::vector<uint8_t>> ||
            std::is_same_v<T, std::vector<char16_t>> ||
            std::is_same_v<T, std::vector<int16_t>> ||
            std::is_same_v<T, std::vector<int32_t>>) {
            if (auto int_value = std::get_if<int32_t>(&value); int_value) {
                vec[static_cast<size_t>(index)] = static_cast<typename T::value_type>(*int_value);
            }
            else {
                throw std::invalid_argument("ArrayData: value type mismatch");
            }
        }
        else {
            vec[static_cast<size_t>(index)] = std::get<typename T::value_type>(value);
        }
    }, elements);
}

InstanceArrayData::InstanceArrayData(Class& type, int32_t length) :
    element_type(type),
    elements(static_cast<size_t>(length), nullptr) {}
