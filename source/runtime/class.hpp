#pragma once
#include "class_file.hpp"

#include <span>
#include <string_view>

struct Field {
    uint16_t access_flags;
    uint16_t name_index;
    uint16_t descriptor_index;
};

struct Method {
    uint16_t access_flags;
    std::string_view name;
    std::string_view descriptor;
    uint16_t max_stack;
    uint16_t max_locals;
    std::span<const std::byte> code;
    std::span<const ExceptionTableEntry> exception_table;
};

class Class {
public:
    explicit Class(const char* filename);

    std::string_view name() const;
    std::string_view super_name() const;

    std::span<const Method> methods() const noexcept { return methods_; }
    const Method* find_method(std::string_view name, std::string_view descriptor) const noexcept;

    Class(const Class&) = delete;
    Class& operator=(const Class&) = delete;
private:
    ClassFile class_file_;
    std::vector<Method> methods_;
};
