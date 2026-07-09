#pragma once
#include "class_loader/class_loader.hpp"
#include "runtime/heap.hpp"
#include "runtime/interpreter.hpp"

#include <filesystem>
#include <string>
#include <string_view>

class Class;

class VM {
public:
    explicit VM(std::string_view main_class);

    void run();

    void initialize_class(Class& cls);

    ClassLoader& class_loader() noexcept { return class_loader_; }
    Interpreter& interpreter() noexcept { return interpreter_; }
    Heap& heap() noexcept { return heap_; }

    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;
private:
    std::string main_class_;
    ClassLoader class_loader_;
    Heap heap_;
    Interpreter interpreter_;
};
