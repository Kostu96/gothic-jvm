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
    std::string_view name;
    std::string_view descriptor;
    uint16_t slot;
};

struct StaticField {
    std::string_view name;
    std::string_view descriptor;
    Value value;
};

struct Method {
    bool is_native;
    std::string_view name;
    std::string_view descriptor;
    uint16_t max_stack;
    uint16_t max_locals;
    std::span<const std::byte> code;
    std::span<const ExceptionTableEntry> exception_table;
    std::function<void(VM&, Frame&)> native_callback;
};

enum class ClassInitState {
    Loaded,        // parsed, but <clinit> not yet started
    Initializing,  // <clinit> currently running on some thread
    Initialized,   // <clinit> completed normally (or class has none)
    Failed         // <clinit> threw; further use must rethrow NoClassDefFoundError
};

enum class ClassKind {
    File,    // parsed from a .class file
    Array    // synthetic array class (e.g. "[Ljava/lang/String;")
};

class Class {
public:
    Class(const char* filename, ClassLoader& class_loader);
    // Synthetic array class. `component` is the element class (null for a
    // primitive base element such as `[I`).
    Class(std::string array_name, Class* component);

    ClassKind kind() const noexcept { return kind_; }
    bool is_array() const noexcept { return kind_ == ClassKind::Array; }
    Class* component_type() const noexcept { return component_; }

    std::string_view this_name() const;
    std::string_view super_name() const;

    size_t get_total_field_count() const noexcept { return total_field_count_; }
    std::optional<uint16_t> find_field_slot(std::string_view name, std::string_view descriptor) const noexcept;
    Value* find_static_field(std::string_view name, std::string_view descriptor) noexcept;
    const Method* find_method(std::string_view name, std::string_view descriptor) const noexcept;

    Value resolve_constant(VM& vm, uint16_t constant_pool_index) const;
    std::string_view resolve_class_name(uint16_t constant_pool_index) const;
    FieldAndMethodRef resolve_field_ref(uint16_t constant_pool_index) const;
    FieldAndMethodRef resolve_method_ref(uint16_t constant_pool_index) const;

    ClassInitState init_state() const noexcept { return init_state_; }
    void set_init_state(ClassInitState state) noexcept { init_state_ = state; }

    Class(const Class&) = delete;
    Class& operator=(const Class&) = delete;
private:
    std::unique_ptr<ClassFile> class_file_;
    std::vector<RuntimeConstantPoolEntry> runtime_constant_pool_;
    mutable std::string_view this_name_;
    mutable std::string_view super_name_;
    size_t total_field_count_ = 0;
    std::vector<Field> fields_;
    std::vector<StaticField> static_fields_;
    std::vector<Method> methods_;
    ClassInitState init_state_ = ClassInitState::Loaded;
    ClassKind kind_ = ClassKind::File;
    std::string name_;        // backing storage for native/array class names
    Class* component_ = nullptr; // element class for array kinds
};
