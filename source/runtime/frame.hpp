#pragma once
#include "runtime/value.hpp"

#include <cstddef>
#include <vector>

class Class;
struct Method;

class Frame {
public:
    Frame(Class& owner, const Method& method);

    Frame(Frame&&) = default;

    Class& owner() const noexcept { return owner_; }
    const Method& method() const noexcept { return method_; }

    std::vector<Value>& locals() noexcept { return locals_; }
    const std::vector<Value>& locals() const noexcept { return locals_; }

    std::vector<Value>& operand_stack() noexcept { return operand_stack_; }
    const std::vector<Value>& operand_stack() const noexcept { return operand_stack_; }

    size_t pc() const noexcept { return pc_; }
    void set_pc(size_t pc) noexcept { pc_ = pc; }
    void branch(size_t offset) noexcept { pc_ += offset; }
    void rewind_pc() noexcept { pc_ = last_pc_; }

    void record_last_pc() noexcept { last_pc_ = pc_; }
    size_t last_pc() const noexcept { return last_pc_; }

    std::uint8_t pop_code_u8();
    std::uint16_t pop_code_u16();
    std::int32_t pop_code_i32();

    void push_stack(Value value) { operand_stack_.push_back(value); }
    Value pop_stack();
    Value peek_stack(uint16_t index = 0) const { return operand_stack_[operand_stack_.size() - 1 - index]; }

    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;
private:
    Class& owner_;
    const Method& method_;
    std::vector<Value> locals_;
    std::vector<Value> operand_stack_;
    size_t pc_ = 0;
    size_t last_pc_ = 0;
};
