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

enum class ConstantTag : u8 {
    Utf8               = 1,
    Integer            = 3,
    Float              = 4,
    Long               = 5,
    Double             = 6,
    Class              = 7,
    String             = 8,
    FieldRef           = 9,
    MethodRef          = 10,
    InterfaceMethodRef = 11,
    NameAndType        = 12
};

struct ConstPoolInfo {
    union {
        struct {
            const char* ptr;
            u16 length;
        } utf8Info;
        struct {
            u32 value;
        } integerInfo;
        struct {
            u8 bytes[4];
        } floatInfo;
        struct {
            u64 value;
        } longInfo;
        struct {
            u8 bytes[8];
        } doubleInfo;
        struct {
            u16 nameIndex;
        } classInfo;
        struct {
            u16 stringIndex;
        } stringInfo;
        struct {
            u16 classIndex;
            u16 nameAndTypeIndex;
        } refInfo;
        struct {
            u16 nameIndex;
            u16 descriptorIndex;
        } nameAndTypeInfo;

        struct {
            u16 u16Field;
        } generic2;
        struct {
            u16 u16Field1;
            u16 u16Field2;
        } generic22;
    };
    ConstantTag tag;
};

namespace AccessFlags {
    constexpr u16 PUBLIC    = 0x0001;
    constexpr u16 PRIVATE   = 0x0002;
    constexpr u16 PROTECTED = 0x0004;
    constexpr u16 STATIC    = 0x0008;
    constexpr u16 FINAL     = 0x0010;
    constexpr u16 SUPER     = 0x0020;
    constexpr u16 INTERFACE = 0x0200;
    constexpr u16 ABSTRACT  = 0x0400;
}

struct AttributeInfo {
    u8* info;
    u32 length;
    u16 nameIndex;
};

struct FieldAndMethodInfo {
    u16 accessFlags;
    u16 nameIndex;
    u16 descriptorIndex;
    u16 attributeCount;
    AttributeInfo* attributes;
};

struct ClassFile {
    ConstPoolInfo* constPool = nullptr;
    u16* interfaces;
    FieldAndMethodInfo* fields = nullptr;
    FieldAndMethodInfo* methods = nullptr;
    AttributeInfo* attributes = nullptr;
    u32 magic;
    u16 verMinor;
    u16 verMajor;
    u16 constPoolCount;
    u16 accessFlags;
    u16 thisClass;
    u16 superClass;
    u16 interfacesCount;
    u16 fieldsCount;
    u16 methodsCount;
    u16 attributesCount;
};
