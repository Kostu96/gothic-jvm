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

    std::optional<Value> execute(Class& owner,
                                 const Method& method,
                                 std::span<const Value> args = {});

    Interpreter(const Interpreter&) = delete;
    Interpreter& operator=(const Interpreter&) = delete;
private:
    void invoke(Class& owner, const Method& method, size_t arg_count, Frame& frame);

    std::optional<Value> run(Frame& frame);

    VM& vm_;
};
