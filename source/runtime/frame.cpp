#include "runtime/frame.hpp"

#include "runtime/class.hpp"
#include "frame.hpp"

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

Value Frame::pop_stack() {
    Value value = operand_stack_.back();
    operand_stack_.pop_back();
    
    return value;
}
