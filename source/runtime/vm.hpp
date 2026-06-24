#pragma once
#include "runtime/class_loader.hpp"
#include "runtime/interpreter.hpp"

#include <filesystem>
#include <string>
#include <string_view>

class Class;

class VM {
public:
    explicit VM(std::string_view main_class);

    void add_classpath_entry(std::filesystem::path dir);

    void run();

    Class* load_class(std::string_view binary_name);

    void initialize_class(Class* cls);

    ClassLoader& class_loader() noexcept { return class_loader_; }
    Interpreter& interpreter() noexcept { return interpreter_; }

    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;
private:
    std::string main_class_;
    ClassLoader class_loader_;
    Interpreter interpreter_;
};
