#pragma once
#include <cstdint>

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <unordered_map>
#include <cassert>

// TEMP //

namespace AccessFlags {
    constexpr u16 PUBLIC = 0x0001;
    constexpr u16 PRIVATE = 0x0002;
    constexpr u16 PROTECTED = 0x0004;
    constexpr u16 STATIC = 0x0008;
    constexpr u16 FINAL = 0x0010;
    constexpr u16 SUPER = 0x0020;
    constexpr u16 INTERFACE = 0x0200;
    constexpr u16 ABSTRACT = 0x0400;
}

union Value
{
    u32 integer;
    bool boolean;
    void* reference;
};
