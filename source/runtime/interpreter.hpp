#pragma once
#include "runtime/value.hpp"

#include <optional>
#include <span>

class Class;
class Frame;
class Thread;
class VM;
struct Method;

class Interpreter {
public:
    explicit Interpreter(VM& vm) noexcept : vm_(vm) {}

    void run(Thread& thread, size_t num_instructions);

    Interpreter(const Interpreter&) = delete;
    Interpreter& operator=(const Interpreter&) = delete;
private:
    void virtual_dispatch(Thread& thread, const Method& resolved_method);
    void invoke(Thread& thread, const Method& method);
    void dispatch_pending_exception(Thread& thread);

    VM& vm_;
};
