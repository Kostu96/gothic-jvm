#include "vm.hpp"
#include "class.hpp"

VM::VM()
{
    m_classPool.addToClassPath("../../../misc/gothic3thebeginning"); // TODO(Kostu): temp
    m_classPool.addToClassPath("../../../misc/classes"); // TODO(Kostu): temp

    m_classPool.loadClass(*this, "java/lang/Object");
    Class& stringClass = *m_classPool.loadClass(*this, "java/lang/String");
    stringClass.prepare();
    stringClass.initialize();

}

void VM::run(const char* className)
{
    Class& class_ = *m_classPool.loadClass(*this, className);
    class_.prepare();
    class_.initialize();
}

void VM::invokeMethod(Class& class_, const Method& method, std::initializer_list<Value> parameters)
{
    CallFrame callFrame(class_, method, parameters);
    
}

Class* VM::loadClass(std::string_view name)
{
    return m_classPool.loadClass(*this, name);
}
