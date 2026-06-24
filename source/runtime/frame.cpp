#include "runtime/frame.hpp"

#include "runtime/class.hpp"
#include "frame.hpp"

Frame::Frame(Class* owner, const Method* method) :
    owner_(owner),
    method_(method),
    locals_(method ? method->max_locals : 0u)
{
    if (method) {
        operand_stack_.reserve(method->max_stack);
    }
}

std::byte Frame::pop_code_byte() {
    return method_->code[pc_++];
}

Value Frame::pop_stack() {
    Value value = operand_stack_.back();
    operand_stack_.pop_back();
    
    return value;
}
