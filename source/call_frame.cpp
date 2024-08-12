#include "call_frame.hpp"
#include "class.hpp"

CallFrame::CallFrame(Class& clazz, const Method& method, std::initializer_list<Value> arguments) :
    m_class{ clazz },
    m_method{ method },
    m_localVariables{ arguments }
{}

void CallFrame::bipush(u8 byte)
{
    m_operandStack.push_back({ .integer = (i32)byte });
}

void CallFrame::iload(u8 index)
{
    m_operandStack.push_back(m_localVariables[index]);
}

void CallFrame::aload(u8 index)
{
    m_operandStack.push_back(m_localVariables[index]);
}

void CallFrame::iadd()
{
    Value b = m_operandStack.back();
    m_operandStack.pop_back();
    m_operandStack.back().integer += b.integer;
}

void CallFrame::getstatic(u16 index)
{
    const auto& c = m_class.getConstant(index - 1);
}

void CallFrame::putstatic(u16 index)
{

}
