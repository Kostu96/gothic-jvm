#pragma once
#include "common.hpp"

class CallFrame
{
public:
    CallFrame() = default;

    void pushLocal(Value value) { m_localVariables.push_back(value); }

    void aload(u8 index);
    void in
private:
    std::vector<Value> m_localVariables;
    std::vector<Value> m_operandStack;
};
