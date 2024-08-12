#pragma once
#include "common.hpp"

namespace OpCode {
    constexpr u8 iconst_1      = 0x04;

    constexpr u8 bipush        = 0x10;

    constexpr u8 ldc           = 0x12;

    constexpr u8 dup           = 0x59;

    constexpr u8 iadd          = 0x60;

    constexpr u8 iload_0       = 0x1A;
    constexpr u8 iload_1       = 0x1B;
    constexpr u8 iload_2       = 0x1C;
    constexpr u8 iload_3       = 0x1D;

    constexpr u8 aload_0       = 0x2A;
    constexpr u8 aload_1       = 0x2B;
    constexpr u8 aload_2       = 0x2C;
    constexpr u8 aload_3       = 0x2D;

    constexpr u8 ireturn       = 0xAC;

    constexpr u8 return_       = 0xB1;
    constexpr u8 getstatic     = 0xB2;
    constexpr u8 putstatic     = 0xB3;

    constexpr u8 invokevirtual = 0xB6;
    constexpr u8 invokespecial = 0xB7;
    constexpr u8 invokestatic  = 0xB8;

    constexpr u8 new_          = 0xBB;

    constexpr u8 ifnonnull     = 0xC7;
}
