#pragma once
#include "class_file.hpp"

#include <span>
#include <string_view>

struct Field {
    uint16_t access_flags;
    uint16_t name_index;
    uint16_t descriptor_index;
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

// JVM spec §5.5 class initialization state.
enum class ClassInitState {
    Loaded,        // parsed, but <clinit> not yet started
    Initializing,  // <clinit> currently running on some thread
    Initialized,   // <clinit> completed normally (or class has none)
    Failed         // <clinit> threw; further use must rethrow NoClassDefFoundError
};

class Class {
public:
    explicit Class(const char* filename);

    std::string_view name() const;
    std::string_view super_name() const;

    std::span<const Method> methods() const noexcept { return methods_; }
    const Method* find_method(std::string_view name, std::string_view descriptor) const noexcept;

    ClassInitState init_state() const noexcept { return init_state_; }
    void set_init_state(ClassInitState state) noexcept { init_state_ = state; }

    Class(const Class&) = delete;
    Class& operator=(const Class&) = delete;
private:
    ClassFile class_file_;
    std::vector<Method> methods_;
    ClassInitState init_state_ = ClassInitState::Loaded;
};
