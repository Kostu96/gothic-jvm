#pragma once
#include "runtime/value.hpp"

#include <optional>
#include <span>

class Class;
class Frame;
class VM;
struct Method;

class Interpreter {
public:
    explicit Interpreter(VM& vm) noexcept : vm_(vm) {}

    std::optional<Value> execute(const Method& method,
                                 std::span<const Value> args = {});

    Interpreter(const Interpreter&) = delete;
    Interpreter& operator=(const Interpreter&) = delete;
private:
    void invoke(const Method& method, Frame& frame);

    std::optional<Value> run(Frame& frame);

    VM& vm_;
};
