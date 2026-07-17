#include "runtime/class.hpp"

#include "class_loader/class_file.hpp"
#include "runtime/vm.hpp"

#include <algorithm>
#include <print>

namespace {

constexpr uint16_t ACC_STATIC =    0x0008;
constexpr uint16_t ACC_SUPER =     0x0020;
constexpr uint16_t ACC_INTERFACE = 0x0200;
constexpr uint16_t ACC_NATIVE =    0x0100;

std::vector<uint8_t> compute_arg_slot_widths(std::string_view descriptor, bool is_static) {
    std::vector<uint8_t> widths;
    if (!is_static) {
        widths.push_back(1);
    }

    size_t i = descriptor.find('(');
    if (i == std::string_view::npos) {
        return widths;
    }
    ++i; // skip '('

    while (i < descriptor.size() && descriptor[i] != ')') {
        bool is_array = false;
        while (i < descriptor.size() && descriptor[i] == '[') {
            is_array = true;
            ++i;
        }
        if (i >= descriptor.size()) {
            break; // malformed descriptor
        }

        const char type = descriptor[i];
        if (type == 'L') {
            i = descriptor.find(';', i);
            if (i == std::string_view::npos) {
                break; // malformed descriptor
            }
            ++i;
        }
        else {
            ++i;
        }

        widths.push_back(!is_array && (type == 'J' || type == 'D') ? 2 : 1);
    }
    return widths;
}

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

}

Class::Class(const char* filename, ClassLoader& class_loader) :
    kind_(Kind::Ordinary),
    class_file_(std::make_unique<ClassFile>(filename)),
    treat_super_specially_((class_file_->access_flags() & ACC_SUPER) != 0),
    is_interface_((class_file_->access_flags() & ACC_INTERFACE) != 0),
    this_name_(class_file_->this_name())
{
    runtime_constant_pool_.resize(class_file_->constant_pool().size());
    for (size_t i = 1; i < class_file_->constant_pool().size(); ++i) {
        std::visit([&](auto&& info) {
            using T = std::decay_t<decltype(info)>;
            if constexpr (std::is_same_v<T, IntegerInfo>) {
                runtime_constant_pool_[i] = RuntimeIntegerInfo{};
            }
            else if constexpr (std::is_same_v<T, LongInfo>) {
                runtime_constant_pool_[i] = RuntimeLongInfo{};
                ++i;
            }
            else if constexpr (std::is_same_v<T, ClassInfo>) {
                runtime_constant_pool_[i] = RuntimeClassInfo{};
            }
            else if constexpr (std::is_same_v<T, StringInfo>) {
                runtime_constant_pool_[i] = RuntimeStringInfo{};
            }
            else if constexpr (std::is_same_v<T, FieldRefInfo>) {
                runtime_constant_pool_[i] = RuntimeFieldRefInfo{};
            }
            else if constexpr (std::is_same_v<T, MethodRefInfo>) {
                runtime_constant_pool_[i] = RuntimeMethodRefInfo{};
            }
        }, class_file_->constant_pool()[i]);
    }

    if (uint16_t super_index = class_file_->super_class(); super_index != 0) {
        auto super_name = class_file_->get_class_name(super_index);
        super_ = &class_loader.load(super_name);
    }

    interfaces_.reserve(class_file_->interfaces().size());
    for (auto interface_index : class_file_->interfaces()) {
        auto interface_name = class_file_->get_class_name(interface_index);
        interfaces_.push_back(&class_loader.load(interface_name));
    }

    auto field_infos = class_file_->fields_info();
    uint16_t slot = super_ ? super_->instance_field_count() : 0;
    for (auto& field_info : field_infos) {
        Field field{ .owner = *this };
        field.is_static = (field_info.access_flags & ACC_STATIC) != 0;
        field.name = class_file_->constant_pool_utf8(field_info.name_index);
        field.descriptor = class_file_->constant_pool_utf8(field_info.descriptor_index);
        if (field.is_static) {
            field.value = default_field_value(field.descriptor);
        }
        else {
            field.slot = slot++;
        }
        fields_.push_back(std::move(field));
    }
    instance_field_count_ = slot;

    auto method_infos = class_file_->methods_info();
    methods_.reserve(method_infos.size());
    for (auto& method_info : method_infos) {
        Method method{ .owner = *this };
        method.name = class_file_->constant_pool_utf8(method_info.name_index);
        method.descriptor = class_file_->constant_pool_utf8(method_info.descriptor_index);
        method.is_static = (method_info.access_flags & ACC_STATIC) != 0;
        method.is_native = (method_info.access_flags & ACC_NATIVE) != 0;
        method.is_class_initializer =
            (method.name == "<clinit>" && method.descriptor == "()V" && method.is_static);
        method.arg_slot_widths = compute_arg_slot_widths(method.descriptor, method.is_static);

        if (method.is_class_initializer && is_interface_) {
            throw std::runtime_error("Class: interface " + std::string(this_name_) +
                " has a class initializer <clinit>, which is not supported");
        }

        if (method.is_native) {
            method.native_callback =
                class_loader.native_methods().find(this_name(), method.name, method.descriptor);
        }

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
}

Class::Class(std::string name, Class* component_type) :
    kind_(component_type ? Kind::Array : Kind::Primitive),
    init_state_(InitState::Initialized),
    class_file_(nullptr),
    treat_super_specially_(false),
    is_interface_(false),
    this_name_(std::move(name)),
    component_type_(component_type) {}

void Class::ensure_initialized(Thread& thread) {
    switch (init_state_) {
    case InitState::Loaded: {
        init_state_ = InitState::Initializing;
        if (const Method* clinit = find_method("<clinit>", "()V")) {
            thread.push_frame(*clinit, {});
        }
        else {
            init_state_ = InitState::Initialized;
        }

        if (super_) {
            super_->ensure_initialized(thread);
        }
    } return;
    case InitState::Initializing:
    case InitState::Initialized:
        return;
    case InitState::Failed:
        throw std::runtime_error(
            "Class: class '" + std::string(this_name_) + "' previously failed initialization");
    }
}

Value Class::resolve_constant(uint16_t index, ClassLoader& class_loader, Heap& heap) {
    return std::visit([&, this](auto&& info) -> Value {
        using T = std::decay_t<decltype(info)>;
        if constexpr (std::is_same_v<T, RuntimeIntegerInfo>) {
            return class_file_->constant_pool_integer(index);
        }
        else if constexpr (std::is_same_v<T, RuntimeLongInfo>) {
            return class_file_->constant_pool_long(index);
        }
        else if constexpr (std::is_same_v<T, RuntimeClassInfo>) {
            auto& cls = resolve_class(index, class_loader);
            return heap.class_object_for(cls);
        }
        else if constexpr (std::is_same_v<T, RuntimeStringInfo>) {
            if (!info.resolved) {
                info.resolved = heap.new_interned_string(class_file_->get_string(index));
            }
            return info.resolved;
        }
        else {
            throw std::runtime_error(std::format(
                "Class: constant pool entry at index {} is not valid constant", index));
        }
    }, runtime_constant_pool_[index]);

    Value value = class_file_->get_constant(index);
    if (std::holds_alternative<std::monostate>(value)) {
        const std::string_view str = class_file_->get_string(index);

        value = heap.new_interned_string(str);
    }

    return value;
}

Class& Class::resolve_class(uint16_t index, ClassLoader& class_loader) {
    auto class_info = std::get_if<RuntimeClassInfo>(&runtime_constant_pool_[index]);
    if (!class_info) {
        throw std::runtime_error(std::format(
            "Class: constant pool entry at index {}  is not a class reference", index));
    }

    if (!class_info->resolved) {
        auto class_name = class_file_->get_class_name(index);
        class_info->resolved = &class_loader.load(class_name);
    }

    return *class_info->resolved;
}

Field& Class::resolve_field(uint16_t index, ClassLoader& class_loader) {
    auto rt_field_ref_info = std::get_if<RuntimeFieldRefInfo>(&runtime_constant_pool_[index]);
    if (!rt_field_ref_info) {
        throw std::runtime_error("Class: constant pool entry at index " +
                                 std::to_string(index) + " is not a field reference");
    }

    if (!rt_field_ref_info->resolved) {
        auto field_ref_info = class_file_->constant_pool_field_ref_info(index);
        auto name_and_type = class_file_->get_name_and_type(field_ref_info.name_and_type_index);
        auto& field_class = resolve_class(field_ref_info.class_index, class_loader);

        auto find_field_in_superinterfaces = [&](this const auto& self, Class* interface) -> Field* {
            Field* field = interface->find_field(name_and_type.first, name_and_type.second);
            if (field != nullptr) {
                return field;
            }
            for (Class* super_interface : interface->interfaces_) {
                field = self(super_interface);
                if (field != nullptr) {
                    return field;
                }
            }
            return nullptr;
        };

        auto find_field_in_superclasses = [&](Class* cls) -> Field* {
            while (cls != nullptr) {
                Field* field = cls->find_field(name_and_type.first, name_and_type.second);
                if (field != nullptr) {
                    return field;
                }
                cls = cls->super_;
            }
            return nullptr;
        };

        Field* field = field_class.find_field(name_and_type.first, name_and_type.second);
        if (field == nullptr) {
            for (Class* interface : field_class.interfaces_) {
                field = find_field_in_superinterfaces(interface);
                if (field != nullptr) {
                    break;
                }
            }
        }
        if (field == nullptr) {
            field = find_field_in_superclasses(field_class.super_);
        }

        if (field == nullptr) {
            throw std::runtime_error("Class: field " + std::string(name_and_type.first) +
                                     " not found in hierarchy of class " + std::string(this_name()));
        }

        rt_field_ref_info->resolved = field;
    }

    return *rt_field_ref_info->resolved;
}

const Method& Class::resolve_method(uint16_t index, ClassLoader& class_loader) {
    auto rt_method_ref_info = std::get_if<RuntimeMethodRefInfo>(&runtime_constant_pool_[index]);
    if (!rt_method_ref_info) {
        throw std::runtime_error("Class: constant pool entry at index " +
                                 std::to_string(index) + " is not a method reference");
    }

    if (!rt_method_ref_info->resolved) {
        auto method_ref_info = class_file_->constant_pool_method_ref_info(index);
        auto name_and_type = class_file_->get_name_and_type(method_ref_info.name_and_type_index);
        auto& method_class = resolve_class(method_ref_info.class_index, class_loader);

        auto find_method_in_superclasses = [&](Class* cls) -> const Method* {
            while (cls != nullptr) {
                const Method* method = cls->find_method(name_and_type.first, name_and_type.second);
                if (method != nullptr) {
                    return method;
                }
                cls = cls->super_;
            }
            return nullptr;
        };

        auto find_method_in_superinterfaces = [&](this const auto& self, Class* interface) -> const Method* {
            const Method* method = interface->find_method(name_and_type.first, name_and_type.second);
            if (method != nullptr) {
                return method;
            }
            for (Class* super_interface : interface->interfaces_) {
                method = self(super_interface);
                if (method != nullptr) {
                    return method;
                }
            }
            return nullptr;
        };

        const Method* method = find_method_in_superclasses(&method_class);
        if (method == nullptr) {
            for (Class* interface : method_class.interfaces_) {
                method = find_method_in_superinterfaces(interface);
                if (method != nullptr) {
                    break;
                }
            }
        }

        if (method == nullptr) {
            throw std::runtime_error(std::format("Class: method {}.{}{} not found.",
                method_class.this_name(), name_and_type.first, name_and_type.second));
        }

        rt_method_ref_info->resolved = method;
    }

    return *rt_method_ref_info->resolved;
}

Field* Class::find_field(std::string_view name, std::string_view descriptor) noexcept {
    auto it = std::ranges::find_if(fields_, [&](const Field& f) {
        return f.name == name && f.descriptor == descriptor;
    });
    return it != fields_.end() ? &*it : nullptr;
}

const Method* Class::find_method(std::string_view name, std::string_view descriptor) const noexcept {
    auto it = std::ranges::find_if(methods_, [&](const Method& m) {
        return m.name == name && m.descriptor == descriptor;
    });
    return it != methods_.end() ? &*it : nullptr;
}
