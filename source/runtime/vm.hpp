#pragma once
#include "class_loader/class_loader.hpp"
#include "runtime/heap.hpp"
#include "runtime/interpreter.hpp"
#include "runtime/native_methods.hpp"

#include <atomic>
#include <filesystem>
#include <string>
#include <string_view>

class Class;
class Display;

// Thrown out of the interpreter loop to unwind a running MIDlet when the host
// window is closed (see VM::request_stop). Caught by the JVM thread in main.
struct VmStopRequested {};

class VM {
public:
    VM();

    void run(std::string_view main_class);

    void initialize_class(Class& cls);

    ClassLoader& class_loader() noexcept { return class_loader_; }
    Interpreter& interpreter() noexcept { return interpreter_; }
    Heap& heap() noexcept { return heap_; }

    // The window/renderer the MIDlet draws onto. Non-owning; may be null when
    // the VM runs headless (e.g. in tests). Native methods reach the screen
    // through here.
    void set_display(Display* display) noexcept { display_ = display; }
    Display* display() const noexcept { return display_; }

    // Cooperative shutdown: the interpreter loop checks stop_requested() each
    // instruction and throws VmStopRequested when set, so a MIDlet that never
    // returns from startApp() can still be torn down when the window closes.
    void request_stop() noexcept { stop_requested_.store(true, std::memory_order_relaxed); }
    bool stop_requested() const noexcept { return stop_requested_.load(std::memory_order_relaxed); }

    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;
private:
    NativeMethods native_methods_;
    ClassLoader class_loader_;
    Heap heap_;
    Interpreter interpreter_;
    Display* display_ = nullptr;
    std::atomic<bool> stop_requested_{ false };
};
