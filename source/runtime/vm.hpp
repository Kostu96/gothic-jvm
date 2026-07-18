#pragma once
#include "class_loader/class_loader.hpp"
#include "platform/record_store_manager.hpp"
#include "runtime/heap.hpp"
#include "runtime/interpreter.hpp"
#include "runtime/native_methods.hpp"
#include "runtime/thread.hpp"

#include <atomic>
#include <filesystem>
#include <string>
#include <string_view>

class Display;

struct VmStopRequested {};

class VM {
public:
    VM();

    void set_main_class(std::string_view main_class) { main_class_name_ = main_class; }

    void set_display(Display* display) noexcept { display_ = display; }
    Display* display() const noexcept { return display_; }

    void run();

    ClassLoader& class_loader() noexcept { return class_loader_; }
    Interpreter& interpreter() noexcept { return interpreter_; }
    Heap& heap() noexcept { return heap_; }
    RecordStoreManager& record_store_manager() noexcept { return record_store_manager_; }

    void request_stop() noexcept { stop_requested_.store(true, std::memory_order_relaxed); }
    bool stop_requested() const noexcept { return stop_requested_.load(std::memory_order_relaxed); }

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
    std::atomic<bool> stop_requested_{ false };
};
