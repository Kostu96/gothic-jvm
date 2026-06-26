#pragma once
#include "class_loader/class_file.hpp"
#include "runtime/value.hpp"

#include <memory>
#include <span>
#include <string_view>
#include <vector>

struct Field {
    std::string_view name;
    std::string_view descriptor;
    Value value;
};

struct Method {
    uint16_t access_flags;
    std::string_view name;
    std::string_view descriptor;
    uint16_t max_stack;
    uint16_t max_locals;
    std::span<const std::byte> code;
    std::span<const ExceptionTableEntry> exception_table;
};

enum class ClassInitState {
    Loaded,        // parsed, but <clinit> not yet started
    Initializing,  // <clinit> currently running on some thread
    Initialized,   // <clinit> completed normally (or class has none)
    Failed         // <clinit> threw; further use must rethrow NoClassDefFoundError
};

class Class {
public:
    Class();

    explicit Class(const char* filename);

    std::string_view this_name() const;
    std::string_view super_name() const;

    std::span<const Method> methods() const noexcept { return methods_; }
    const Method* find_method(std::string_view name, std::string_view descriptor) const noexcept;

    Value* find_static_field(std::string_view name, std::string_view descriptor) noexcept;
    FieldAndMethodRef resolve_field_ref(uint16_t constant_pool_index) const;
    std::string_view resolve_class_name(uint16_t constant_pool_index) const;

    ClassInitState init_state() const noexcept { return init_state_; }
    void set_init_state(ClassInitState state) noexcept { init_state_ = state; }

    Class(const Class&) = delete;
    Class& operator=(const Class&) = delete;
private:
    std::unique_ptr<ClassFile> class_file_;
    mutable std::string_view this_name_;
    mutable std::string_view super_name_;
    std::vector<Method> methods_;
    std::vector<Field> static_fields_;
    ClassInitState init_state_ = ClassInitState::Loaded;
};
