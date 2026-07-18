#pragma once
#include "class_loader/constant_pool_entry.hpp"
#include "runtime/value.hpp"

#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

struct FieldAndMethodStringRef {
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

    const std::vector<ConstantPoolEntry>& constant_pool() const noexcept { return constant_pool_; }

    uint16_t this_class() const noexcept { return this_class_; }
    uint16_t super_class() const noexcept { return super_class_; }
    const std::vector<uint16_t>& interfaces() const noexcept { return interfaces_; }

    // TODO(Kostu): move to ConstantPool class
    std::string_view constant_pool_utf8(uint16_t index) const { return constant_pool_at<Utf8Info>(index).value; }
    int32_t constant_pool_integer(uint16_t index) const { return constant_pool_at<IntegerInfo>(index).value; }
    int64_t constant_pool_long(uint16_t index) const { return constant_pool_at<LongInfo>(index).value; }
    FieldRefInfo constant_pool_field_ref_info(uint16_t index) const { return constant_pool_at<FieldRefInfo>(index); }
    MethodRefInfo constant_pool_method_ref_info(uint16_t index) const { return constant_pool_at<MethodRefInfo>(index); }

    std::string_view get_class_name(uint16_t constant_pool_index) const;
    std::string_view get_string(uint16_t constant_pool_index) const;
    std::pair<std::string_view, std::string_view> get_name_and_type(uint16_t constant_pool_index) const;

    FieldAndMethodStringRef get_field_string_ref(uint16_t constant_pool_index) const;
    FieldAndMethodStringRef get_method_string_ref(uint16_t constant_pool_index) const;
    FieldAndMethodStringRef get_interface_method_string_ref(uint16_t constant_pool_index) const;

    uint16_t access_flags() const { return access_flags_; }
    std::string_view this_name() const { return get_class_name(this_class_); }
    std::string_view super_name() const { return super_class_ != 0 ? get_class_name(super_class_) : ""; }

    std::span<const FieldAndMethodInfo> fields_info() const noexcept { return fields_info_; }
    std::span<const FieldAndMethodInfo> methods_info() const noexcept { return methods_info_; }

    ClassFile(const ClassFile&) = delete;
    ClassFile& operator=(const ClassFile&) = delete;
private:
    template<typename T>
    const T& constant_pool_at(uint16_t index) const {
        if (index == 0 || index >= constant_pool_.size()) {
            throw std::out_of_range("ClassFile: invalid constant pool index: " + std::to_string(index));
        }

        if (auto entry = std::get_if<T>(&constant_pool_[index])) {
            return *entry;
        }

        throw std::runtime_error("ClassFile: constant pool entry at index " + std::to_string(index) + " is not of the expected type");
    }

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
