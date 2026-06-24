#pragma once
#include "runtime/value.hpp"

#include <cstddef>
#include <vector>

class Class;
struct Method;

class Frame {
public:
    Frame(Class* owner, const Method* method);

    Class* owner() const noexcept { return owner_; }
    const Method* method() const noexcept { return method_; }

    std::vector<Value>& locals() noexcept { return locals_; }
    const std::vector<Value>& locals() const noexcept { return locals_; }

    std::vector<Value>& operand_stack() noexcept { return operand_stack_; }
    const std::vector<Value>& operand_stack() const noexcept { return operand_stack_; }

    size_t pc() const noexcept { return pc_; }
    void set_pc(size_t pc) noexcept { pc_ = pc; }

    std::byte pop_code_byte();

    void push_stack(Value value) { operand_stack_.push_back(value); }
    Value pop_stack();
    Value peek_stack() const { return operand_stack_.back(); }

    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;
private:
    Class* owner_;
    const Method* method_;
    std::vector<Value> locals_;
    std::vector<Value> operand_stack_;
    size_t pc_ = 0;
};
