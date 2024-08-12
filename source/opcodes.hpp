#pragma once
#include "common.hpp"

namespace OpCode {
    constexpr u8 iconst_1      = 0x04;

    constexpr u8 ldc           = 0x12;

    constexpr u8 dup           = 0x59;

    constexpr u8 aload_0       = 0x2A;

    constexpr u8 return_       = 0xB1;
    constexpr u8 getstatic     = 0xB2;
    constexpr u8 putstatic     = 0xB3;

    constexpr u8 invokevirtual = 0xB6;
    constexpr u8 invokespecial = 0xB7;
    constexpr u8 invokestatic  = 0xB8;

    constexpr u8 new_          = 0xBB;

    constexpr u8 ifnonnull     = 0xC7;
}
