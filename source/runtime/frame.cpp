#include "runtime/frame.hpp"

#include "runtime/class.hpp"

Frame::Frame(Class& owner, const Method& method) :
    owner_(owner),
    method_(method),
    locals_( method.max_locals)
{
    operand_stack_.reserve(method.max_stack);
}

std::uint8_t Frame::pop_code_u8() {
    return std::to_integer<uint8_t>(method_.code[pc_++]);
}

std::uint16_t Frame::pop_code_u16() {
    const auto high = std::to_integer<std::uint16_t>(method_.code[pc_++]);
    const auto low = std::to_integer<std::uint16_t>(method_.code[pc_++]);
    return (high << 8) | low;
}

std::int32_t Frame::pop_code_i32() {
    const auto b1 = std::to_integer<std::int32_t>(method_.code[pc_++]);
    const auto b2 = std::to_integer<std::int32_t>(method_.code[pc_++]);
    const auto b3 = std::to_integer<std::int32_t>(method_.code[pc_++]);
    const auto b4 = std::to_integer<std::int32_t>(method_.code[pc_++]);
    return (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
}

Value Frame::pop_stack() {
    Value value = operand_stack_.back();
    operand_stack_.pop_back();
    
    return value;
}
