#pragma once
#include "class_loader/class_file.hpp"
#include "runtime/runtime_constant_pool_entry.hpp"
#include "runtime/value.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

class ClassLoader;
class Frame;
class VM;

struct Field {
    Class& owner;
    bool is_static;
    std::string_view name;
    std::string_view descriptor;
    Value value;
    uint16_t slot;
};

struct Method {
    Class& owner;
    bool is_static;
    bool is_native;
    std::string_view name;
    std::string_view descriptor;
    uint16_t max_stack;
    uint16_t max_locals;
    std::span<const std::byte> code;
    std::span<const ExceptionTableEntry> exception_table;
    std::function<void(VM&, Frame&)> native_callback;
};


class Class {
public:
    enum class InitState {
        Loaded,        // parsed, but <clinit> not yet started
        Initializing,  // <clinit> currently running on some thread
        Initialized,   // <clinit> completed normally (or class has none)
        Failed         // <clinit> threw; further use must rethrow NoClassDefFoundError
    };

    enum class Kind {
        Ordinary,
        Array,
        Primitive
    };

    Class(const char* filename, ClassLoader& class_loader);
    explicit Class(std::string name, Class* component_type = nullptr);

    InitState init_state() const noexcept { return init_state_; }
    void set_init_state(InitState state) noexcept { init_state_ = state; }

    bool treat_super_specially() const noexcept { return treat_super_specially_; }
    bool is_interface() const noexcept { return is_interface_; }

    Class* super() const { return super_; }
    Class* component_type() const noexcept { return component_type_; }
    bool is_primitive() const noexcept { return kind_ == Kind::Primitive; }

    std::string_view this_name() const { return this_name_; }
    std::string_view super_name() const { return super_ ? super_->this_name() : ""; }

    size_t instance_field_count() const noexcept { return instance_field_count_; }

    Class& resolve_class(uint16_t index, ClassLoader& class_loader);
    Field& resolve_field(uint16_t index, ClassLoader& class_loader);
    const Method& resolve_method(uint16_t index, ClassLoader& class_loader);

    Field* find_field(std::string_view name, std::string_view descriptor) noexcept;
    const Method* find_method(std::string_view name, std::string_view descriptor) const noexcept;

    Object* create_string(VM& vm, std::string_view str) const;
    Value resolve_constant(VM& vm, uint16_t constant_pool_index) const;
    std::string_view resolve_class_name(uint16_t constant_pool_index) const;
    FieldAndMethodStringRef resolve_method_ref(uint16_t constant_pool_index) const;

    Class(const Class&) = delete;
    Class& operator=(const Class&) = delete;
private:
    Kind kind_;
    InitState init_state_ = InitState::Loaded;
    const std::unique_ptr<const ClassFile> class_file_;
    const bool treat_super_specially_;
    const bool is_interface_;
    std::vector<RuntimeConstantPoolEntry> runtime_constant_pool_;
    Class* super_ = nullptr;
    std::vector<Class*> interfaces_;
    size_t instance_field_count_ = 0;
    std::vector<Field> fields_;
    std::vector<Method> methods_;
    std::string this_name_;
    Class* component_type_ = nullptr;
};
