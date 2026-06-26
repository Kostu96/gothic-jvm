#include "runtime/class.hpp"

#include "class_loader/class_file.hpp"

#include <algorithm>

namespace {

constexpr uint16_t ACC_STATIC = 0x0008;

Value default_field_value(std::string_view descriptor) {
    if (!descriptor.empty()) {
        switch (descriptor.front()) {
        case 'J': return static_cast<int64_t>(0);
        case 'F': return 0.0f;
        case 'D': return 0.0;
        case 'L':
        case '[': return static_cast<Object*>(nullptr);
        default:  break; // B, C, I, S, Z all use a 32-bit zero
        }
    }

    return static_cast<int32_t>(0);
}

} // namespace

Class::Class() {

}

Class::Class(const char* filename) :
    class_file_(std::make_unique<ClassFile>(filename))
{
    uint16_t methods_count = class_file_->get_methods_count();
    methods_.reserve(methods_count);

    for (uint16_t i = 0; i < methods_count; ++i) {
        Method method{};
        method.access_flags = class_file_->get_method_access_flags(i);
        method.name = class_file_->get_method_name(i);
        method.descriptor = class_file_->get_method_descriptor(i);

        uint16_t attributes_count = class_file_->get_method_attributes_count(i);
        for (uint16_t j = 0; j < attributes_count; ++j) {
            if (const auto* code = class_file_->get_method_attribute_code(i, j)) {
                method.max_stack = code->max_stack;
                method.max_locals = code->max_locals;
                method.code = std::span<const std::byte>{ code->code };
                method.exception_table = std::span<const ExceptionTableEntry>{ code->exception_table };
                break;
            }
        }

        methods_.push_back(std::move(method));
    }

    uint16_t fields_count = class_file_->get_fields_count();
    for (uint16_t i = 0; i < fields_count; ++i) {
        if ((class_file_->get_field_access_flags(i) & ACC_STATIC) == 0) {
            continue;
        }

        Field field{};
        field.name = class_file_->get_field_name(i);
        field.descriptor = class_file_->get_field_descriptor(i);
        field.value = default_field_value(field.descriptor);
        static_fields_.push_back(field);
    }
}

std::string_view Class::name() const {
    return class_file_->get_this_name();
}

std::string_view Class::super_name() const {
    return class_file_->get_super_name();
}

const Method* Class::find_method(std::string_view name, std::string_view descriptor) const noexcept {
    auto it = std::ranges::find_if(methods_, [&](const Method& m) {
        return m.name == name && m.descriptor == descriptor;
    });
    return it != methods_.end() ? &*it : nullptr;
}

Value* Class::find_static_field(std::string_view name, std::string_view descriptor) noexcept {
    auto it = std::ranges::find_if(static_fields_, [&](const Field& f) {
        return f.name == name && f.descriptor == descriptor;
    });
    return it != static_fields_.end() ? &it->value : nullptr;
}

FieldRef Class::resolve_field_ref(uint16_t constant_pool_index) const {
    return class_file_->get_field_ref(constant_pool_index);
}

std::string_view Class::resolve_class_name(uint16_t constant_pool_index) const {
    return class_file_->get_class_name(constant_pool_index);
}
