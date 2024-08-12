#pragma once
#include "common.hpp"

class Class;
struct Method;

class CallFrame
{
public:
    CallFrame(Class& clazz, const Method& method, std::initializer_list<Value> arguments);

    void bipush(u8 byte);
    void iload(u8 index);
    void aload(u8 index);
    void iadd();
    void getstatic(u16 index);
    void putstatic(u16 index);
private:
    Class& m_class;
    const Method& m_method;

    std::vector<Value> m_localVariables;
    std::vector<Value> m_operandStack;
};
