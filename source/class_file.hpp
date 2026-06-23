#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

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
	explicit ClassFile(const char* path);

    std::string_view get_class_name(uint16_t index) const;
    std::string_view get_this_name() const;
    std::string_view get_super_name() const;

    uint16_t get_methods_count() const { return static_cast<uint16_t>(methods_.size()); }
    std::string_view get_method_name(uint16_t index) const;
    std::string_view get_method_descriptor(uint16_t index) const;
    uint16_t get_method_access_flags(uint16_t index) const;
    uint16_t get_method_attributes_count(uint16_t method_index) const;
    std::string_view get_method_attribute_name(uint16_t method_index, uint16_t attribute_index) const;
    const CodeAttributeInfo* get_method_attribute_code(uint16_t method_index, uint16_t attribute_index) const;

    ClassFile(const ClassFile&) = delete;
    ClassFile& operator=(const ClassFile&) = delete;
private:
    std::string_view get_utf8(uint16_t index) const;

    uint16_t version_minor_;
    uint16_t version_major_;
    std::vector<ConstantPoolEntry> constant_pool_;
    uint16_t access_flags_;
    uint16_t this_class_;
    uint16_t super_class_;
    std::vector<uint16_t> interfaces_;
    std::vector<FieldAndMethodInfo> fields_;
    std::vector<FieldAndMethodInfo> methods_;

    /*u16 m_attributesCount;
    AttributeInfo* m_attributes = nullptr;*/
};
