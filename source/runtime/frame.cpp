#include "runtime/frame.hpp"

#include "runtime/class.hpp"

Frame::Frame(Class* owner, const Method* method) :
    owner_(owner),
    method_(method),
    locals_(method ? method->max_locals : 0u)
{
    if (method) {
        operand_stack_.reserve(method->max_stack);
    }
}
