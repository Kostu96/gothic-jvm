#include "runtime/class.hpp"

#include "class_loader/class_file.hpp"
#include "runtime/vm.hpp"

#include <algorithm>

// TODO(Kostu): temp:
#include "runtime/frame.hpp"

namespace {

constexpr uint16_t ACC_STATIC = 0x0008;
constexpr uint16_t ACC_NATIVE = 0x0100;

Value default_field_value(std::string_view descriptor) {
    if (!descriptor.empty()) {
        switch (descriptor.front()) {
        case 'J': return static_cast<int64_t>(0);
        case 'F': return 0.0f;
        case 'D': return 0.0;
        case 'L':
        case '[': return static_cast<RuntimeObject*>(nullptr);
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

void java_lang_class_for_name(VM& vm, Frame& frame) {
    auto* key_obj = std::get<RuntimeObject*>(frame.pop_stack());
    if (key_obj == nullptr) {
        // In a complete VM this would raise NullPointerException.
        throw std::runtime_error("forName: class name is null");
    }

    auto& str_obj = std::get<InstanceData>(key_obj->data);
    auto* chars = std::get<RuntimeObject*>(str_obj.fields[0]);
    auto& char_array = std::get<PrimitiveArrayData>(chars->data);

    std::string cpp_str;
    for (auto c : char_array.elements) {
        cpp_str.push_back(static_cast<char>(std::get<int32_t>(c)));
    }
    if (cpp_str == "com.sun.midp.midlet.scheduler") {
        cpp_str = "com/sun/midp/midlet/Scheduler";
    }

    Class* cls = vm.class_loader().load(cpp_str);
    auto* cls_obj = vm.heap().class_object_for(*cls);
    frame.push_stack(cls_obj);
}

void java_lang_class_new_instance(VM& vm, Frame& frame) {
    auto* cls_obj = std::get<RuntimeObject*>(frame.pop_stack());
    if (cls_obj == nullptr) {
        // In a complete VM this would raise NullPointerException.
        throw std::runtime_error("newInstance: class is null");
    }
    auto& mirror = std::get<ClassMirrorData>(cls_obj->data);
    Class* cls = mirror.mirrored;
    RuntimeObject* instance = vm.heap().new_instance(*cls);
    cls->find_method("<init>", "()V");
    Value val = instance;
    vm.interpreter().execute(*cls, *cls->find_method("<init>", "()V"), std::span{ &val, 1 });

    frame.push_stack(instance);
}

void get_property0(VM& vm, Frame& frame) {
    auto* key_obj = std::get<RuntimeObject*>(frame.pop_stack());
    if (key_obj == nullptr) {
        // In a complete VM this would raise NullPointerException.
        throw std::runtime_error("getProperty0: key is null");
    }
    
    auto& str_obj = std::get<InstanceData>(key_obj->data);
    auto* chars = std::get<RuntimeObject*>(str_obj.fields[0]);
    auto& char_array = std::get<PrimitiveArrayData>(chars->data);

    std::string cpp_str;
    for (auto c : char_array.elements) {
        cpp_str.push_back(static_cast<char>(std::get<int32_t>(c)));
    }

    std::printf("get_property0 asked for: %s\n", cpp_str.c_str());

    //temp:
    frame.push_stack(key_obj);
}

std::unordered_map<std::string, std::function<void(VM&, Frame&)>> native_method_callbacks = {
    { "java/lang/System.currentTimeMillis()J", java_lang_system_current_time_millis },
    { "java/lang/Class.forName(Ljava/lang/String;)Ljava/lang/Class;", java_lang_class_for_name },
    { "java/lang/Class.newInstance()Ljava/lang/Object;", java_lang_class_new_instance },
    { "com/sun/midp/main/Configuration.getProperty0(Ljava/lang/String;)Ljava/lang/String;", get_property0 }
};

} // namespace

Class::Class(const char* filename, ClassLoader& class_loader) :
    class_file_(std::make_unique<ClassFile>(filename))
{
    auto super_name_str = super_name();
    Class* super = super_name_str.empty() ? nullptr : class_loader.load(super_name_str);

    auto field_infos = class_file_->get_fields_info();
    uint16_t slot = super ? super->get_total_field_count() : 0;
    for (auto& field_info : field_infos) {
        if ((field_info.access_flags & ACC_STATIC) == 0) {
            Field field{};
            field.name = class_file_->get_utf8(field_info.name_index);
            field.descriptor = class_file_->get_utf8(field_info.descriptor_index);
            field.slot = slot++;
            fields_.push_back(std::move(field));
        }
        else {
            StaticField field{};
            field.name = class_file_->get_utf8(field_info.name_index);
            field.descriptor = class_file_->get_utf8(field_info.descriptor_index);
            field.value = default_field_value(field.descriptor);
            static_fields_.push_back(std::move(field));
        }
    }
    total_field_count_ = slot;

    auto method_infos = class_file_->get_methods_info();
    methods_.reserve(method_infos.size());
    for (auto& method_info : method_infos) {
        Method method{};
        method.is_native = (method_info.access_flags & ACC_NATIVE) != 0;
        method.name = class_file_->get_utf8(method_info.name_index);
        method.descriptor = class_file_->get_utf8(method_info.descriptor_index);

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

Class::Class(std::string array_name, Class* component) :
    class_file_(nullptr),
    init_state_(ClassInitState::Initialized), // array classes have no <clinit>
    kind_(ClassKind::Array),
    name_(std::move(array_name)),
    component_(component)
{}

std::string_view Class::this_name() const {
    if (kind_ != ClassKind::File) {
        return name_;
    }
    if (this_name_.empty()) {
        this_name_ = class_file_->get_this_name();
    }
    return this_name_;
}

std::string_view Class::super_name() const {
    if (kind_ == ClassKind::Array) {
        return "java/lang/Object"; // arrays inherit directly from Object
    }
    if (super_name_.empty()) {
        super_name_ = class_file_->get_super_name();
    }
    return super_name_;
}

std::optional<uint16_t> Class::find_field_slot(std::string_view name, std::string_view descriptor) const noexcept {
    auto it = std::ranges::find_if(fields_, [&](const Field& f) {
        return f.name == name && f.descriptor == descriptor;
    });
    return it != fields_.end() ? std::optional(it->slot) : std::nullopt;
}

Value* Class::find_static_field(std::string_view name, std::string_view descriptor) noexcept {
    auto it = std::ranges::find_if(static_fields_, [&](const StaticField& f) {
        return f.name == name && f.descriptor == descriptor;
    });
    return it != static_fields_.end() ? &it->value : nullptr;
}

const Method* Class::find_method(std::string_view name, std::string_view descriptor) const noexcept {
    auto it = std::ranges::find_if(methods_, [&](const Method& m) {
        return m.name == name && m.descriptor == descriptor;
    });
    return it != methods_.end() ? &*it : nullptr;
}

Value Class::resolve_constant(VM& vm, uint16_t constant_pool_index) const {
    Value value = class_file_->get_constant(constant_pool_index);

    if (auto obj = std::get_if<RuntimeObject*>(&value); obj && *obj == nullptr) {
        const std::string_view str = class_file_->get_string(constant_pool_index);

        auto string_class = vm.load_class("java/lang/String");
        RuntimeObject* string_obj = vm.heap().new_instance(*string_class);

        // Backing char[] for the String. The constant is stored as (modified)
        // UTF-8; for ASCII each byte maps directly to one char.
        RuntimeObject* chars =
            vm.heap().new_primitive_array(ElementType::Char, static_cast<int32_t>(str.size()));
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
            if (const auto slot = string_class->find_field_slot(name, descriptor)) {
                instance.fields[*slot] = field_value;
            }
        };
        set_field("value", "[C", chars);
        set_field("offset", "I", static_cast<int32_t>(0));
        set_field("count", "I", static_cast<int32_t>(str.size()));
        set_field("hash", "I", static_cast<int32_t>(0));

        value = string_obj;
    }

    return value;
}

std::string_view Class::resolve_class_name(uint16_t constant_pool_index) const {
    return class_file_->get_class_name(constant_pool_index);
}

FieldAndMethodRef Class::resolve_field_ref(uint16_t constant_pool_index) const {
    return class_file_->get_field_ref(constant_pool_index);
}

FieldAndMethodRef Class::resolve_method_ref(uint16_t constant_pool_index) const {
    return class_file_->get_method_ref(constant_pool_index);
}
