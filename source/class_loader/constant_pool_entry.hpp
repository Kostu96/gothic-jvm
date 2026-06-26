#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

struct Utf8Info {
    std::string value;
};

struct IntegerInfo {
    int32_t value;
};

struct LongInfo {
    int64_t value;
};

struct ClassInfo {
    uint16_t name_index;
};

struct StringInfo {
    uint16_t string_index;
};

struct FieldRefInfo {
    uint16_t class_index;
    uint16_t name_and_type_index;
};

struct MethodRefInfo {
    uint16_t class_index;
    uint16_t name_and_type_index;
};

struct InterfaceMethodRefInfo {
    uint16_t class_index;
    uint16_t name_and_type_index;
};

struct NameAndTypeInfo {
    uint16_t name_index;
    uint16_t descriptor_index;
};

using ConstantPoolEntry = std::variant<
    std::monostate,
    Utf8Info,
    IntegerInfo,
    LongInfo,
    ClassInfo,
    StringInfo,
    FieldRefInfo,
    MethodRefInfo,
    InterfaceMethodRefInfo,
    NameAndTypeInfo
>;
