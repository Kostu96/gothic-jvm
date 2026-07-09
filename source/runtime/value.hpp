#pragma once
#include <cstdint>
#include <variant>

struct Object;

using Value = std::variant<
    std::monostate, // unset slot
    int32_t,        // boolean, byte, char, short, int
    int64_t,        // long
    float,          // float
    double,         // double
    Object*         // reference (nullptr == null)
>;
