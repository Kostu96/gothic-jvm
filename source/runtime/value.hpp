#pragma once
#include <cstdint>
#include <variant>

struct RuntimeObject;

using Value = std::variant<
    std::monostate, // unset slot
    int32_t,        // boolean, byte, char, short, int
    int64_t,        // long
    float,          // float
    double,         // double
    RuntimeObject*  // reference (nullptr == null)
>;
