#include "runtime/class.hpp"

#include "class_loader/class_file.hpp"
#include "runtime/vm.hpp"

#include <algorithm>
#include <print>

// TODO(Kostu): temp:
#include "runtime/frame.hpp"

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

// TODO(Kostu): temp:
void java_lang_system_current_time_millis(VM& vm, Frame& frame) {
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    frame.push_stack(static_cast<int64_t>(ms.count()));
}

//void java_lang_class_new_instance(VM& vm, Frame& frame) {
//    auto* cls_obj = std::get<Object*>(frame.pop_stack());
//    if (cls_obj == nullptr) {
//        // In a complete VM this would raise NullPointerException.
//        throw std::runtime_error("newInstance: class is null");
//    }
//    auto& mirror = std::get<ClassMirrorData>(cls_obj->data);
//    Class* cls = mirror.mirrored;
//    Object* instance = vm.heap().new_instance(*cls);
//    cls->find_method("<init>", "()V");
//    Value val = instance;
//    vm.interpreter().execute(*cls, *cls->find_method("<init>", "()V"), std::span{ &val, 1 });
//
//    frame.push_stack(instance);
//}

void java_lang_object_get_class(VM& vm, Frame& frame) {
    auto* obj = std::get<Object*>(frame.pop_stack());

    auto& instance = std::get<InstanceData>(obj->data);
    auto* class_obj = vm.heap().class_object_for(instance.type);
    frame.push_stack(class_obj);
}

void java_lang_class_get_name(VM& vm, Frame& frame) {
    auto* cls_obj = std::get<Object*>(frame.pop_stack());
    auto& mirror = std::get<ClassMirrorData>(cls_obj->data);
    Class& cls = mirror.mirrored;
    Object* str_obj = cls.create_string(vm, cls.this_name());
    frame.push_stack(str_obj);
}

void javax_microedition_lcdui_canvas_get_width(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
    frame.push_stack(static_cast<int32_t>(240)); // TODO(Kostu): temp: hardcoded width
}

void javax_microedition_lcdui_canvas_get_height(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
    frame.push_stack(static_cast<int32_t>(320)); // TODO(Kostu): temp: hardcoded height
}

void java_lang_string_char_at(VM& vm, Frame& frame) {
    auto index = std::get<int32_t>(frame.pop_stack());
    auto* str_obj = std::get<Object*>(frame.pop_stack());
    auto& str_instance = std::get<InstanceData>(str_obj->data);
    auto* chars_obj = std::get<Object*>(str_instance.fields[0]);
    auto& char_array = std::get<PrimitiveArrayData>(chars_obj->data);
    if (index < 0 || index >= char_array.length()) {
        // In a complete VM this would raise StringIndexOutOfBoundsException.
        throw std::runtime_error("charAt: index out of bounds");
    }
    auto char_value = char_array.get(index);
    frame.push_stack(char_value);
}

void java_lang_string_index_of(VM& vm, Frame& frame) {
    auto index = std::get<int32_t>(frame.pop_stack());
    auto char_value = std::get<int32_t>(frame.pop_stack());
    auto* str_obj = std::get<Object*>(frame.pop_stack());
    auto& str_instance = std::get<InstanceData>(str_obj->data);
    auto* chars_obj = std::get<Object*>(str_instance.fields[0]);
    auto& char_array = std::get<PrimitiveArrayData>(chars_obj->data);
    int32_t result_index = -1;
    for (int32_t i = index; i < char_array.length(); ++i) {
        if (std::get<int32_t>(char_array.get(i)) == char_value) {
            result_index = i;
            break;
        }
    }
    frame.push_stack(result_index);
}

std::unordered_map<std::string, std::function<void(VM&, Frame&)>> native_method_callbacks = {
    { "java/lang/System.currentTimeMillis()J", java_lang_system_current_time_millis },
    { "javax/microedition/lcdui/Canvas.getWidth()I", javax_microedition_lcdui_canvas_get_width },
    { "javax/microedition/lcdui/Canvas.getHeight()I", javax_microedition_lcdui_canvas_get_height },
    { "java/lang/Object.getClass()Ljava/lang/Class;", java_lang_object_get_class },
    { "java/lang/Class.getName()Ljava/lang/String;", java_lang_class_get_name },
    { "java/lang/String.charAt(I)C", java_lang_string_char_at },
    { "java/lang/String.indexOf(II)I", java_lang_string_index_of }
};

} // namespace

Class::Class(const char* filename, ClassLoader& class_loader) :
    kind_(Kind::Ordinary),
    class_file_(std::make_unique<ClassFile>(filename)),
    treat_super_specially_((class_file_->access_flags() & ACC_SUPER) != 0),
    is_interface_((class_file_->access_flags() & ACC_INTERFACE) != 0),
    this_name_(class_file_->this_name())
{
    runtime_constant_pool_.resize(class_file_->constant_pool().size());
    for (size_t i = 1; i < class_file_->constant_pool().size(); ++i) {
        if (std::holds_alternative<LongInfo>(class_file_->constant_pool()[i])) {
            ++i;
        }
        if (std::holds_alternative<ClassInfo>(class_file_->constant_pool()[i])) {
            runtime_constant_pool_[i] = RuntimeClassInfo{};
        }
        if (std::holds_alternative<FieldRefInfo>(class_file_->constant_pool()[i])) {
            runtime_constant_pool_[i] = RuntimeFieldRefInfo{};
        }
        if (std::holds_alternative<MethodRefInfo>(class_file_->constant_pool()[i])) {
            runtime_constant_pool_[i] = RuntimeMethodRefInfo{};
        }
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
        method.is_static = (method_info.access_flags & ACC_STATIC) != 0;
        method.is_native = (method_info.access_flags & ACC_NATIVE) != 0;
        method.name = class_file_->constant_pool_utf8(method_info.name_index);
        method.descriptor = class_file_->constant_pool_utf8(method_info.descriptor_index);
        method.arg_slot_widths = compute_arg_slot_widths(method.descriptor, method.is_static);
        method.num_args = static_cast<uint16_t>(method.arg_slot_widths.size());

        if (method.is_native) {
            std::string key = std::string(this_name()) + "." + std::string(method.name) + std::string(method.descriptor);
            auto it = native_method_callbacks.find(key);
            if (it != native_method_callbacks.end()) {
                method.native_callback = it->second;
            }
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

Class& Class::resolve_class(uint16_t index, ClassLoader& class_loader) {
    auto class_info = std::get_if<RuntimeClassInfo>(&runtime_constant_pool_[index]);
    if (!class_info) {
        throw std::runtime_error("Class: constant pool entry at index " + std::to_string(index) + " is not a class reference");
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
            throw std::runtime_error("Class: method " + std::string(name_and_type.first) +
                                     " not found in hierarchy of class " + std::string(this_name()));
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

Object* Class::create_string(VM& vm, std::string_view str) const {
    auto& string_class = vm.class_loader().load("java/lang/String");
    Object* string_obj = vm.heap().new_instance(string_class);

    // Backing char[] for the String. The constant is stored as (modified)
    // UTF-8; for ASCII each byte maps directly to one char.
    Object* chars =
        vm.heap().new_primitive_array(PrimitiveArrayData::ElementType::Char, static_cast<int32_t>(str.size()));
    auto& char_array = std::get<PrimitiveArrayData>(chars->data);
    for (size_t i = 0; i < str.size(); ++i) {
        char_array.set(static_cast<int32_t>(i),
            static_cast<int32_t>(static_cast<unsigned char>(str[i])));
    }

    // Populate the String fields directly instead of running <init>([C)V,
    // the same way HotSpot materializes interned literals. This avoids
    // depending on System.arraycopy and leaves no uninitialized field slots.
    auto& instance = std::get<InstanceData>(string_obj->data);
    const auto set_field = [&](std::string_view name, std::string_view descriptor, Value field_value) {
        if (auto field = string_class.find_field(name, descriptor)) {
            instance.fields[field->slot] = field_value;
        }
        };
    set_field("value", "[C", chars);
    set_field("offset", "I", static_cast<int32_t>(0));
    set_field("count", "I", static_cast<int32_t>(str.size()));
    set_field("hash", "I", static_cast<int32_t>(0));

    return string_obj;
}

Value Class::resolve_constant(VM& vm, uint16_t constant_pool_index) const {
    Value value = class_file_->get_constant(constant_pool_index);
    if (std::holds_alternative<std::monostate>(value)) {
        const std::string_view str = class_file_->get_string(constant_pool_index);

        value = create_string(vm, str);
    }

    return value;
}
