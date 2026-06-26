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
    auto method_infos = class_file_->get_methods_info();
    methods_.reserve(method_infos.size());
    for (auto& method_info : method_infos) {
        Method method{};
        method.access_flags = method_info.access_flags;
        method.name = class_file_->get_utf8(method_info.name_index);
        method.descriptor = class_file_->get_utf8(method_info.descriptor_index);

        for (auto& attr : method_info.attributes) {
            if (const auto* code = std::get_if<CodeAttributeInfo>(&attr.info)) {
                method.max_stack = code->max_stack;
                method.max_locals = code->max_locals;
                method.code = std::span<const std::byte>{ code->code };
                method.exception_table = std::span<const ExceptionTableEntry>{ code->exception_table };
                break;
            }
        }

        methods_.push_back(std::move(method));
    }

    auto field_infos = class_file_->get_fields_info();
    for (auto& field_info : field_infos) {
        if ((field_info.access_flags & ACC_STATIC) == 0) {
            continue;
        }

        Field field{};
        field.name = class_file_->get_utf8(field_info.name_index);
        field.descriptor = class_file_->get_utf8(field_info.descriptor_index);
        field.value = default_field_value(field.descriptor);
        static_fields_.push_back(field);
    }
}

std::string_view Class::this_name() const {
    if (this_name_.empty()) {
        this_name_ = class_file_->get_this_name();
    }
    return this_name_;
}

std::string_view Class::super_name() const {
    if (super_name_.empty()) {
        super_name_ = class_file_->get_super_name();
    }
    return super_name_;
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

FieldAndMethodRef Class::resolve_field_ref(uint16_t constant_pool_index) const {
    return class_file_->get_field_ref(constant_pool_index);
}

std::string_view Class::resolve_class_name(uint16_t constant_pool_index) const {
    return class_file_->get_class_name(constant_pool_index);
}
