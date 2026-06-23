#include "runtime/class.hpp"

#include "class_file.hpp"

#include <algorithm>

Class::Class(const char* filename) :
    class_file_(filename)
{
    uint16_t methods_count = class_file_.get_methods_count();
    methods_.reserve(methods_count);

    for (uint16_t i = 0; i < methods_count; ++i) {
        Method method{};
        method.access_flags = class_file_.get_method_access_flags(i);
        method.name = class_file_.get_method_name(i);
        method.descriptor = class_file_.get_method_descriptor(i);

        uint16_t attributes_count = class_file_.get_method_attributes_count(i);
        for (uint16_t j = 0; j < attributes_count; ++j) {
            if (const auto* code = class_file_.get_method_attribute_code(i, j)) {
                method.max_stack = code->max_stack;
                method.max_locals = code->max_locals;
                method.code = std::span<const std::byte>{ code->code };
                method.exception_table = std::span<const ExceptionTableEntry>{ code->exception_table };
                break;
            }
        }

        methods_.push_back(std::move(method));
    }
}

std::string_view Class::name() const {
    return class_file_.get_this_name();
}

std::string_view Class::super_name() const {
    return class_file_.get_super_name();
}

const Method* Class::find_method(std::string_view name, std::string_view descriptor) const noexcept {
    auto it = std::ranges::find_if(methods_, [&](const Method& m) {
        return m.name == name && m.descriptor == descriptor;
    });
    return it != methods_.end() ? &*it : nullptr;
}
