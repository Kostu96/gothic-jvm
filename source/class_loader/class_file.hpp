#pragma once
#include "class_loader/constant_pool_entry.hpp"
#include "runtime/value.hpp"

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

struct FieldAndMethodRef {
    std::string_view class_name;
    std::string_view name;
    std::string_view descriptor;
};

struct ExceptionTableEntry {
    uint16_t start_pc;
    uint16_t end_pc;
    uint16_t handler_pc;
    uint16_t catch_type;
};

struct CodeAttributeInfo {
    uint16_t max_stack;
    uint16_t max_locals;
    std::vector<std::byte> code;
    std::vector<ExceptionTableEntry> exception_table;
    std::vector<std::byte> attributes; // Placeholder for nested attributes
};

struct AttributeInfo {
    uint16_t name_index;
    std::variant<std::vector<std::byte>, CodeAttributeInfo> info;
};

struct FieldAndMethodInfo {
    uint16_t access_flags;
    uint16_t name_index;
    uint16_t descriptor_index;
    std::vector<AttributeInfo> attributes;
};

class ClassFile
{
public:
	explicit ClassFile(const char* filename);

    std::string_view get_utf8(uint16_t constant_pool_index) const;
    int32_t get_integer(uint16_t constant_pool_index) const;
    int64_t get_long(uint16_t constant_pool_index) const;
    std::string_view get_class_name(uint16_t constant_pool_index) const;
    std::string_view get_string(uint16_t constant_pool_index) const;
    FieldAndMethodRef get_field_ref(uint16_t constant_pool_index) const;
    FieldAndMethodRef get_method_ref(uint16_t constant_pool_index) const;
    FieldAndMethodRef get_interface_method_ref(uint16_t constant_pool_index) const;

    Value get_constant(uint16_t constant_pool_index) const;

    uint16_t get_access_flags() const { return access_flags_; }
    std::string_view get_this_name() const { return get_class_name(this_class_); }
    std::string_view get_super_name() const { return super_class_ != 0 ? get_class_name(super_class_) : ""; }

    std::span<const FieldAndMethodInfo> get_fields_info() const noexcept { return fields_info_; }
    std::span<const FieldAndMethodInfo> get_methods_info() const noexcept { return methods_info_; }

    ClassFile(const ClassFile&) = delete;
    ClassFile& operator=(const ClassFile&) = delete;
private:
    uint16_t version_minor_;
    uint16_t version_major_;
    std::vector<ConstantPoolEntry> constant_pool_;
    uint16_t access_flags_;
    uint16_t this_class_;
    uint16_t super_class_;
    std::vector<uint16_t> interfaces_;
    std::vector<FieldAndMethodInfo> fields_info_;
    std::vector<FieldAndMethodInfo> methods_info_;

    /*u16 m_attributesCount;
    AttributeInfo* m_attributes = nullptr;*/
};
