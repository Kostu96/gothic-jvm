#pragma once
#include "class_loader/class_loader.hpp"
#include "runtime/heap.hpp"
#include "runtime/interpreter.hpp"
#include "runtime/native_methods.hpp"

#include <filesystem>
#include <string>
#include <string_view>

class Class;

class VM {
public:
    VM();

    void run(std::string_view main_class);

    void initialize_class(Class& cls);

    ClassLoader& class_loader() noexcept { return class_loader_; }
    Interpreter& interpreter() noexcept { return interpreter_; }
    Heap& heap() noexcept { return heap_; }

    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;
private:
    NativeMethods native_methods_;
    ClassLoader class_loader_;
    Heap heap_;
    Interpreter interpreter_;
};
