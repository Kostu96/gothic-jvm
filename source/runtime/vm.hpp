#pragma once
#include "class_loader/class_loader.hpp"
#include "platform/record_store_manager.hpp"
#include "runtime/heap.hpp"
#include "runtime/interpreter.hpp"
#include "runtime/native_methods.hpp"
#include "runtime/thread.hpp"

#include <filesystem>
#include <stop_token>
#include <string>
#include <string_view>

class Display;

class VM {
public:
    VM();

    void set_main_class(std::string_view main_class) { main_class_name_ = main_class; }

    void set_display(Display* display) noexcept { display_ = display; }
    Display* display() const noexcept { return display_; }

    void run(std::stop_token stop_token);

    ClassLoader& class_loader() noexcept { return class_loader_; }
    Interpreter& interpreter() noexcept { return interpreter_; }
    Heap& heap() noexcept { return heap_; }
    RecordStoreManager& record_store_manager() noexcept { return record_store_manager_; }

    Thread& create_thread();

    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;
private:
    std::string main_class_name_;
    Display* display_ = nullptr;
    NativeMethods native_methods_;
    ClassLoader class_loader_;
    Heap heap_;
    RecordStoreManager record_store_manager_;
    std::vector<std::unique_ptr<Thread>> threads_;
    Interpreter interpreter_;
};
