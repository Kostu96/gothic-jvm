#pragma once
#include "common.hpp"

class Class;
struct Method;

class CallFrame
{
public:
    CallFrame(Class& class_, const Method& method, std::initializer_list<Value> arguments);

    Class& getCurrentClass() { return m_class; }

    u8 prevU8() const;
    u8 nextU8();
    u16 nextU16();

    void bipush(u8 byte);
    void iload(u8 index);
    void aload(u8 index);
    void iadd();
    void getstatic(u16 index);
    void putstatic(u16 index);
private:
    Class& m_class;
    const Method& m_method;

    u32 m_pc = 0;
    std::vector<Value> m_localVariables;
    std::vector<Value> m_operandStack;
};
