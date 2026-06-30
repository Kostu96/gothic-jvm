#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

class Class;
class Runtime;
struct RuntimeObject;
struct Field;
struct Method;
struct InterfaceMethod;

struct RuntimeClassInfo {
    Class* resolved = nullptr;
};

struct RuntimeStringInfo {
    RuntimeObject* resolved = nullptr;
};

struct RuntimeFieldRefInfo {
    Field* resolved = nullptr;
};

struct RuntimeMethodRefInfo {
    Method* resolved = nullptr;
};

struct RuntimeInterfaceMethodRefInfo {
    InterfaceMethod* resolved = nullptr;
};

using RuntimeConstantPoolEntry = std::variant<
    std::monostate,
    RuntimeClassInfo,
    RuntimeStringInfo,
    RuntimeFieldRefInfo,
    RuntimeMethodRefInfo,
    RuntimeInterfaceMethodRefInfo
>;
