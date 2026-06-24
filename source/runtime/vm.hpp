#pragma once
#include "runtime/class_loader.hpp"
#include "runtime/interpreter.hpp"

#include <filesystem>
#include <string>
#include <string_view>

class Class;

// Top-level driver. Owns the class loader and interpreter, and drives
// loading, initialization and execution starting from the configured main class.
class VM {
public:
    explicit VM(std::string_view main_class);

    void add_classpath_entry(std::filesystem::path dir);

    // Loads + initializes the main class, then (eventually) invokes
    // main([Ljava/lang/String;)V. The invocation step is a TODO until the
    // interpreter can handle enough opcodes.
    void run();

    // Loads (or returns cached) class by binary name ("java/lang/Object").
    Class* load_class(std::string_view binary_name);

    // Ensures the class is initialized (JVM §5.5). Safe to call repeatedly
    // and re-entrantly from the same thread.
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
