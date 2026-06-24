#pragma once
#include "runtime/value.hpp"

#include <optional>
#include <span>

class Class;
class Frame;
class VM;
struct Method;

// Executes Java bytecode. Logically per-thread; today the VM holds a single
// instance because we are single-threaded.
class Interpreter {
public:
    explicit Interpreter(VM& vm) noexcept : vm_(vm) {}

    // Executes `method` to completion. `args` are copied into the first locals
    // (remaining locals stay default-constructed). Returns the method's return
    // value, or std::nullopt for void methods.
    std::optional<Value> execute(Class* owner,
                                 const Method& method,
                                 std::span<const Value> args = {});

    Interpreter(const Interpreter&) = delete;
    Interpreter& operator=(const Interpreter&) = delete;
private:
    // Dispatch loop. Returns when the current frame returns.
    std::optional<Value> run(Frame& frame);

    VM& vm_;
};
