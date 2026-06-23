#pragma once
#include "common.hpp"
#include "class_pool.hpp"
#include "call_frame.hpp"

class Class;
struct Method;

class VM
{
public:
    VM();

    void run(const char* className);

    void invokeMethod(Class& class_, const Method& method, std::initializer_list<Value> parameters);

    Class* loadClass(std::string_view name);
private:
    ClassPool m_classPool;
    std::vector<CallFrame> m_callStack;
};
