#include "class.hpp"
#include "class_file.hpp"
#include "class_pool.hpp"

#include <cassert>
#include <functional>

Class::Class(ClassPool& classPool, const ClassFile& classFile) noexcept :
    m_classPoolRef{ classPool }
{
    m_constantPoolSize = classFile.m_constantPoolCount - 1;
    m_constantPool = new ConstantPoolEntry[m_constantPoolSize];
    for (u16 i = 0; i < m_constantPoolSize; i++)
    {
        switch (classFile.m_constantPool[i].tag)
        {
        case ConstPoolInfo::Tag::Class:
            m_constantPool[i].symbolicRef1 = classFile.m_constantPool[i].u16Index - 1;
            break;
        }
    }

    m_superClassIndex = (classFile.m_superClass != 0) ?
        m_classPoolRef.loadClass(classFile.getClassName(classFile.m_superClass)) :
        InvalidClassIndex;

    if (classFile.m_interfacesCount > 0) {
        m_interfacesSize = classFile.m_interfacesCount;
        m_interfaces = new u32[m_interfacesSize];
        for (u16 i = 0; i < m_interfacesSize; i++)
        {
            m_interfaces[i] = m_classPoolRef.loadClass(classFile.getClassName(classFile.m_interfaces[i]));
        }
    }

    // TODO(Kostu): fields
    if (classFile.m_fieldsCount > 0) {
        m_fieldsSize = classFile.m_fieldsCount;
        m_fields = new Field[m_fieldsSize];
        for (u16 i = 0; i < m_fieldsSize; i++)
        {
            m_fields[i].accessFlags = classFile.m_fields[i].accessFlags;
            m_fields[i].nameIndex = classFile.m_fields[i].nameIndex - 1;
            m_fields[i].descriptorIndex = classFile.m_fields[i].descriptorIndex - 1;
        }
    }
}

Class::~Class()
{
    delete[] m_fields;
    delete[] m_interfaces;
    delete[] m_constantPool;
}

Class::Class(Class&& other) noexcept :
    m_classPoolRef{ other.m_classPoolRef },
    m_classFile{ other.m_classFile },
    m_constantPool{ other.m_constantPool },
    m_constantPoolSize{ other.m_constantPoolSize },
    m_superClassIndex{ other.m_superClassIndex },
    m_interfaces{ other.m_interfaces },
    m_interfacesSize{ other.m_interfacesSize },
    m_fields{ other.m_fields },
    m_fieldsSize{ other.m_fieldsSize },
    m_staticFields{ std::move(other.m_staticFields) }
{
    other.m_classFile = nullptr;
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
    
}

static void parseFieldDescriptor(const char* desc, u16 length)
{
    std::function<void(const char*, u16)> parse = [&parse](const char* ptr, u16 length) {
        switch (*ptr) {
        case 'B': printf("byte"); break;
        case 'C': printf("char"); break;
        case 'D': printf("double"); break;
        case 'F': printf("float"); break;
        case 'I': printf("int"); break;
        case 'J': printf("long"); break;
        case 'S': printf("short"); break;
        case 'Z': printf("boolean"); break;
        case 'L': {
            printf("ref %.*s", length - 2, ptr + 1);
            ptr += length;
        } break;
        case '[': {
            parse(ptr + 1, length - 1);
            printf("[]");
        } break;
        default:
            printf("unk char - %c", *ptr);
            assert(false);
        }
        };

    parse(desc, length);
}

void Class::prepare()
{
    for (u16 i = 0; i < m_fieldsSize; i++)
    {
        if (m_fields[i].accessFlags & AccessFlags::STATIC) {
            //printf("static field %u: %s\n", i, name.c_str());
        }
    }
}
