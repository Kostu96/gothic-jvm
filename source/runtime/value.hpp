#pragma once
#include <cstdint>
#include <variant>

class Object; // Forward decl: heap-allocated object, added when we build the heap.

// One JVM operand-stack or local-variable slot.
// Note: long/double occupy two slots per the JVM spec; for now we store them
// in a single Value entry and will revisit the layout when we implement
// long/double load/store opcodes.
using Value = std::variant<
    std::monostate, // unset slot
    int32_t,        // boolean, byte, char, short, int
    int64_t,        // long
    float,          // float
    double,         // double
    Object*         // reference (nullptr == null)
>;
