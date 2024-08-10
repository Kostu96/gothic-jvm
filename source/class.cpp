#include "class.hpp"
#include "class_file.hpp"

#include <cassert>

Class::Class(const ClassFile& classFile)
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

    assert(classFile.m_superClass == 0); // not implemented yet
}

Class::~Class()
{
    delete[] m_constantPool;
}
