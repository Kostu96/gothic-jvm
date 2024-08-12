#include "call_frame.hpp"

void CallFrame::aload(u8 index)
{
    m_operandStack.push_back(m_localVariables[index]);
}
