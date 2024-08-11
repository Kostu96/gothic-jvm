#include "class.hpp"
#include "class_file.hpp"
#include "class_pool.hpp"

#include <cassert>

Class::~Class()
{
    delete[] m_fields;
    delete[] m_interfaces;
    delete[] m_constantPool;
}

Class::Class(Class&& other) noexcept :
    m_classPoolRef{ other.m_classPoolRef },
    m_constantPool{ other.m_constantPool },
    m_constantPoolSize{ other.m_constantPoolSize },
    m_superClassIndex{ other.m_superClassIndex },
    m_interfaces{ other.m_interfaces },
    m_interfacesSize{ other.m_interfacesSize },
    m_fields{ other.m_fields },
    m_fieldsSize{ other.m_fieldsSize }
{
    other.m_constantPool = nullptr;
    other.m_constantPoolSize = 0;
    other.m_superClassIndex = InvalidClassIndex;
    other.m_interfaces = nullptr;
    other.m_interfacesSize = 0;
    other.m_fields = nullptr;
    other.m_fieldsSize = 0;
}

void Class::derive(const ClassFile& classFile)
{
    m_constantPoolSize = classFile.m_constantPoolCount - 1;
    m_constantPool = new ConstantPoolEntry[m_constantPoolSize];
    for (u16 i = 0; i < m_constantPoolSize; i++)
    {
        switch (classFile.m_constantPool[i].tag)
        {
        case ConstPoolInfo::Tag::Class:
            m_constantPool[i].symbolicRef1 = classFile.m_constantPool[i].u16Index;
            break;
        }
    }
    
    m_superClassIndex = (classFile.m_superClass != 0) ?
        m_classPoolRef.loadClass(classFile.getClassName(classFile.m_superClass)) :
        InvalidClassIndex;

    if (classFile.m_interfacesCount > 0) {
        m_interfacesSize = classFile.m_interfacesCount;
        m_interfaces = new u32[m_interfacesSize];
        for (u16 i = 0; i < classFile.m_interfacesCount; i++)
        {
            m_interfaces[i] = m_classPoolRef.loadClass(classFile.getClassName(classFile.m_interfaces[i]));
        }
    }

    // TODO(Kostu): fields
}

void Class::prepare()
{

}
