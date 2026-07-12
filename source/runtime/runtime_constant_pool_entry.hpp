#pragma once
#include "runtime/value.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

class Class;
class Runtime;
struct Object;
struct Field;
struct Method;
struct InterfaceMethod;

struct RuntimeIntegerInfo {};

struct RuntimeLongInfo {};

struct RuntimeClassInfo {
    Class* resolved = nullptr;
};

struct RuntimeStringInfo {
    Object* resolved = nullptr;
};

struct RuntimeFieldRefInfo {
    Field* resolved = nullptr;
};

struct RuntimeMethodRefInfo {
    const Method* resolved = nullptr;
};

struct RuntimeInterfaceMethodRefInfo {
    InterfaceMethod* resolved = nullptr;
};

using RuntimeConstantPoolEntry = std::variant<
    std::monostate,
    RuntimeIntegerInfo,
    RuntimeLongInfo,
    RuntimeClassInfo,
    RuntimeStringInfo,
    RuntimeFieldRefInfo,
    RuntimeMethodRefInfo,
    RuntimeInterfaceMethodRefInfo
>;
