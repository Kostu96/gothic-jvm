#include "class_file.hpp"

#include "utils/binary_reader.hpp"

#include <fstream>

namespace {

std::vector<std::byte> read_file(const char* filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Failed to open file: " + std::string(filename));
    }

    file.seekg(0, std::ios::end);
    auto length = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<std::byte> buffer(length);
    file.read(reinterpret_cast<char*>(buffer.data()), length);

    file.close();
    return buffer;
}

}

ClassFile::ClassFile(const char* filename) {
    auto buffer = read_file(filename);
    util::BinaryReader reader(buffer);

    auto magic = reader.read_u32();
    if (magic != 0xCAFEBABE) {
        throw std::runtime_error("Invalid class file: " + std::string(filename));
    }

    version_minor_ = reader.read_u16();
    version_major_ = reader.read_u16();

    auto constant_pool_count = reader.read_u16();
    constant_pool_.resize(constant_pool_count);
    for (uint16_t i = 1; i < constant_pool_count; ++i) {
        uint8_t tag = reader.read_u8();
        switch (tag) {
        case 1: { // Utf8
            uint16_t length = reader.read_u16();
            constant_pool_[i] = Utf8Info{ reader.read_string(length) };
        } break;
        case 3: { // Integer
            constant_pool_[i] = IntegerInfo{ static_cast<int32_t>(reader.read_u32()) };
        } break;
        case 5: { // Long
            uint32_t high_bytes = reader.read_u32();
            uint32_t low_bytes = reader.read_u32();
            constant_pool_[i] = LongInfo{ (static_cast<int64_t>(high_bytes) << 32) | low_bytes };
            ++i; // Long takes up two entries in the constant pool
        } break;
        case 7: { // Class
            constant_pool_[i] = ClassInfo{ reader.read_u16() };
        } break;
        case 8: { // String
            constant_pool_[i] = StringInfo{ reader.read_u16() };
        } break;
        case 9: { // FieldRef
            uint16_t class_index = reader.read_u16();
            uint16_t name_and_type_index = reader.read_u16();
            constant_pool_[i] = FieldRefInfo{ class_index, name_and_type_index };
        } break;
        case 10: { // MethodRef
            uint16_t class_index = reader.read_u16();
            uint16_t name_and_type_index = reader.read_u16();
            constant_pool_[i] = MethodRefInfo{ class_index, name_and_type_index };
        } break;
        case 11: { // InterfaceMethodRef
            uint16_t class_index = reader.read_u16();
            uint16_t name_and_type_index = reader.read_u16();
            constant_pool_[i] = InterfaceMethodRefInfo{ class_index, name_and_type_index };
        } break;
        case 12: { // NameAndType
            uint16_t name_index = reader.read_u16();
            uint16_t descriptor_index = reader.read_u16();
            constant_pool_[i] = NameAndTypeInfo{ name_index, descriptor_index };
        } break;
        default:
            throw std::runtime_error("Unsupported constant pool tag: " + std::to_string(tag));
        }
    }

    access_flags_ = reader.read_u16();
    this_class_ = reader.read_u16();
    super_class_ = reader.read_u16();

    auto interfaces_count = reader.read_u16();
    interfaces_.resize(interfaces_count);
    for (uint16_t i = 0; i < interfaces_count; ++i) {
        interfaces_[i] = reader.read_u16();
    }

    auto read_attributes = [this, &reader]() {
        std::vector<AttributeInfo> attributes;
        uint16_t count = reader.read_u16();
        attributes.resize(count);
        for (uint16_t j = 0; j < count; ++j) {
            attributes[j].name_index = reader.read_u16();
            uint32_t length = reader.read_u32();

            if (get_utf8(attributes[j].name_index) == "Code") {
                CodeAttributeInfo code_info;
                code_info.max_stack = reader.read_u16();
                code_info.max_locals = reader.read_u16();
                uint32_t code_length = reader.read_u32();
                code_info.code = reader.read_bytes(code_length);
                uint16_t exception_table_length = reader.read_u16();
                code_info.exception_table.resize(exception_table_length);
                for (uint16_t k = 0; k < exception_table_length; ++k) {
                    code_info.exception_table[k].start_pc = reader.read_u16();
                    code_info.exception_table[k].end_pc = reader.read_u16();
                    code_info.exception_table[k].handler_pc = reader.read_u16();
                    code_info.exception_table[k].catch_type = reader.read_u16();
                }
                uint32_t consumed = 2u + 2u + 4u + code_length + 2u + exception_table_length * 8u;
                if (consumed > length) {
                    throw std::runtime_error("ClassFile: malformed Code attribute (length underflow)");
                }
                code_info.attributes = reader.read_bytes(length - consumed);
                attributes[j].info = std::move(code_info);
            }
            else {
                attributes[j].info = reader.read_bytes(length);
            }
        }
        return attributes;
    };

    auto fields_count = reader.read_u16();
    fields_info_.resize(fields_count);
    for (uint16_t i = 0; i < fields_count; ++i) {
        fields_info_[i].access_flags = reader.read_u16();
        fields_info_[i].name_index = reader.read_u16();
        fields_info_[i].descriptor_index = reader.read_u16();
        fields_info_[i].attributes = read_attributes();
    }

    auto methods_count = reader.read_u16();
    methods_info_.resize(methods_count);
    for (uint16_t i = 0; i < methods_count; ++i) {
        methods_info_[i].access_flags = reader.read_u16();
        methods_info_[i].name_index = reader.read_u16();
        methods_info_[i].descriptor_index = reader.read_u16();
        methods_info_[i].attributes = read_attributes();
    }

    /*m_attributesCount = parseU16BigEndian(ptr);
    m_attributes = new AttributeInfo[m_attributesCount];
    for (u16 i = 0; i < m_attributesCount; i++)
    {
        m_attributes[i].nameIndex = parseU16BigEndian(ptr);
        m_attributes[i].length = parseU32BigEndian(ptr);
        m_attributes[i].info = ptr; ptr += m_attributes[i].length;
    }*/
}

std::string_view ClassFile::get_utf8(uint16_t constant_pool_index) const {
    if (constant_pool_index == 0 || constant_pool_index >= constant_pool_.size()) {
        throw std::out_of_range("ClassFile: invalid constant pool index: " + std::to_string(constant_pool_index));
    }

    const auto* utf8_info = std::get_if<Utf8Info>(&constant_pool_[constant_pool_index]);
    if (!utf8_info) {
        throw std::runtime_error("ClassFile: constant pool entry at index " + std::to_string(constant_pool_index) + " is not a Utf8");
    }

    return utf8_info->value;
}

int32_t ClassFile::get_integer(uint16_t constant_pool_index) const {
    if (constant_pool_index == 0 || constant_pool_index >= constant_pool_.size()) {
        throw std::out_of_range("ClassFile: invalid constant pool index: " + std::to_string(constant_pool_index));
    }

    const auto* integer_info = std::get_if<IntegerInfo>(&constant_pool_[constant_pool_index]);
    if (!integer_info) {
        throw std::runtime_error("ClassFile: constant pool entry at index " + std::to_string(constant_pool_index) + " is not an Integer");
    }

    return integer_info->value;
}

int64_t ClassFile::get_long(uint16_t constant_pool_index) const {
    if (constant_pool_index == 0 || constant_pool_index >= constant_pool_.size()) {
        throw std::out_of_range("ClassFile: invalid constant pool index: " + std::to_string(constant_pool_index));
    }

    const auto* long_info = std::get_if<LongInfo>(&constant_pool_[constant_pool_index]);
    if (!long_info) {
        throw std::runtime_error("ClassFile: constant pool entry at index " + std::to_string(constant_pool_index) + " is not a Long");
    }

    return long_info->value;
}

std::string_view ClassFile::get_class_name(uint16_t constant_pool_index) const {
    if (constant_pool_index == 0 || constant_pool_index >= constant_pool_.size()) {
        throw std::out_of_range("ClassFile: invalid constant pool index: " + std::to_string(constant_pool_index));
    }

    const auto* class_info = std::get_if<ClassInfo>(&constant_pool_[constant_pool_index]);
    if (!class_info) {
        throw std::runtime_error("ClassFile: constant pool entry at index " + std::to_string(constant_pool_index) + " is not a Class");
    }

    return get_utf8(class_info->name_index);
}

std::string_view ClassFile::get_string(uint16_t constant_pool_index) const {
    if (constant_pool_index == 0 || constant_pool_index >= constant_pool_.size()) {
        throw std::out_of_range("ClassFile: invalid constant pool index: " + std::to_string(constant_pool_index));
    }

    const auto* string_info = std::get_if<StringInfo>(&constant_pool_[constant_pool_index]);
    if (!string_info) {
        throw std::runtime_error("ClassFile: constant pool entry at index " + std::to_string(constant_pool_index) +
                                 " is not a String");
    }

    return get_utf8(string_info->string_index);
}

FieldAndMethodRef ClassFile::get_field_ref(uint16_t constant_pool_index) const {
    if (constant_pool_index == 0 || constant_pool_index >= constant_pool_.size()) {
        throw std::out_of_range("ClassFile: invalid constant pool index: " + std::to_string(constant_pool_index));
    }

    const auto* field_ref = std::get_if<FieldRefInfo>(&constant_pool_[constant_pool_index]);
    if (!field_ref) {
        throw std::runtime_error("ClassFile: constant pool entry at index " + std::to_string(constant_pool_index) + " is not a Fieldref");
    }

    if (field_ref->name_and_type_index == 0 || field_ref->name_and_type_index >= constant_pool_.size()) {
        throw std::out_of_range("ClassFile: invalid constant pool index: " + std::to_string(field_ref->name_and_type_index));
    }

    const auto* name_and_type = std::get_if<NameAndTypeInfo>(&constant_pool_[field_ref->name_and_type_index]);
    if (!name_and_type) {
        throw std::runtime_error("ClassFile: constant pool entry at index " + std::to_string(field_ref->name_and_type_index) + " is not a NameAndType");
    }

    return FieldAndMethodRef{
        get_class_name(field_ref->class_index),
        get_utf8(name_and_type->name_index),
        get_utf8(name_and_type->descriptor_index)
    };
}

FieldAndMethodRef ClassFile::get_method_ref(uint16_t constant_pool_index) const {
    if (constant_pool_index == 0 || constant_pool_index >= constant_pool_.size()) {
        throw std::out_of_range("ClassFile: invalid constant pool index: " + std::to_string(constant_pool_index));
    }

    const auto* method_ref = std::get_if<MethodRefInfo>(&constant_pool_[constant_pool_index]);
    if (!method_ref) {
        throw std::runtime_error("ClassFile: constant pool entry at index " + std::to_string(constant_pool_index) + " is not a Methodref");
    }

    if (method_ref->name_and_type_index == 0 || method_ref->name_and_type_index >= constant_pool_.size()) {
        throw std::out_of_range("ClassFile: invalid constant pool index: " + std::to_string(method_ref->name_and_type_index));
    }

    const auto* name_and_type = std::get_if<NameAndTypeInfo>(&constant_pool_[method_ref->name_and_type_index]);
    if (!name_and_type) {
        throw std::runtime_error("ClassFile: constant pool entry at index " + std::to_string(method_ref->name_and_type_index) + " is not a NameAndType");
    }

    return FieldAndMethodRef{
        get_class_name(method_ref->class_index),
        get_utf8(name_and_type->name_index),
        get_utf8(name_and_type->descriptor_index)
    };
}

FieldAndMethodRef ClassFile::get_interface_method_ref(uint16_t constant_pool_index) const {
    if (constant_pool_index == 0 || constant_pool_index >= constant_pool_.size()) {
        throw std::out_of_range("ClassFile: invalid constant pool index: " + std::to_string(constant_pool_index));
    }

    const auto* interface_method_ref = std::get_if<InterfaceMethodRefInfo>(&constant_pool_[constant_pool_index]);
    if (!interface_method_ref) {
        throw std::runtime_error("ClassFile: constant pool entry at index " + std::to_string(constant_pool_index) + " is not an InterfaceMethodref");
    }

    if (interface_method_ref->name_and_type_index == 0 || interface_method_ref->name_and_type_index >= constant_pool_.size()) {
        throw std::out_of_range("ClassFile: invalid constant pool index: " + std::to_string(interface_method_ref->name_and_type_index));
    }

    const auto* name_and_type = std::get_if<NameAndTypeInfo>(&constant_pool_[interface_method_ref->name_and_type_index]);
    if (!name_and_type) {
        throw std::runtime_error("ClassFile: constant pool entry at index " + std::to_string(interface_method_ref->name_and_type_index) + " is not a NameAndType");
    }

    return FieldAndMethodRef{
        get_class_name(interface_method_ref->class_index),
        get_utf8(name_and_type->name_index),
        get_utf8(name_and_type->descriptor_index)
    };
}

Value ClassFile::get_constant(uint16_t constant_pool_index) const {
    if (constant_pool_index == 0 || constant_pool_index >= constant_pool_.size()) {
        throw std::out_of_range("ClassFile: invalid constant pool index: " + std::to_string(constant_pool_index));
    }

    return std::visit([&](const auto& entry) {
        using T = std::decay_t<decltype(entry)>;
        if constexpr (std::is_same_v<T, IntegerInfo> || std::is_same_v<T, LongInfo>) {
            return Value(entry.value);
        }
        else if constexpr (std::is_same_v<T, StringInfo>) {
            auto str = get_utf8(entry.string_index);
            // In a complete VM, this would create a new String object.
            return Value(nullptr);
        }
        else {
            throw std::runtime_error("ClassFile: constant pool entry at index " + std::to_string(constant_pool_index) +
                                     " is not a supported constant type");
            return Value();
        }
        }, constant_pool_[constant_pool_index]);
}
