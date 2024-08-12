#include "class.hpp"
#include "class_file.hpp"
#include "class_pool.hpp"
#include "file_io.hpp"

#include <functional>

Class::Class(ClassPool& classPool, const char* classFilename) noexcept :
    m_classPoolRef{ classPool },
    m_classFile{ new ClassFile(classFilename) }
{
    m_constantPoolSize = m_classFile->m_constantPoolCount - 1;
    m_constantPool = new ConstantPoolEntry[m_constantPoolSize];
    for (u16 i = 0; i < m_constantPoolSize; i++)
    {
        switch (m_classFile->m_constantPool[i].tag)
        {
        case ConstPoolInfo::Tag::Class:
            m_constantPool[i].symbolicRef1 = m_classFile->m_constantPool[i].u16Index - 1;
            break;
        }
    }

    m_superClassIndex = (m_classFile->m_superClass != 0) ?
        m_classPoolRef.loadClass(m_classFile->getClassName(m_classFile->m_superClass)) :
        InvalidClassIndex;

    if (m_classFile->m_interfacesCount > 0) {
        m_interfacesSize = m_classFile->m_interfacesCount;
        m_interfaces = new u32[m_interfacesSize];
        for (u16 i = 0; i < m_interfacesSize; i++)
        {
            m_interfaces[i] = m_classPoolRef.loadClass(m_classFile->getClassName(m_classFile->m_interfaces[i]));
        }
    }

    if (m_classFile->m_fieldsCount > 0) {
        m_fieldsSize = m_classFile->m_fieldsCount;
        m_fields = new Field[m_fieldsSize];
        for (u16 i = 0; i < m_fieldsSize; i++)
        {
            m_fields[i].accessFlags = m_classFile->m_fields[i].accessFlags;
            m_fields[i].nameIndex = m_classFile->m_fields[i].nameIndex - 1;
            m_fields[i].descriptorIndex = m_classFile->m_fields[i].descriptorIndex - 1;
        }
    }

    for (u16 i = 0; i < m_classFile->m_methodsCount; i++)
    {
        Method method{};
        const auto& name = m_classFile->m_constantPool[m_classFile->m_methods[i].nameIndex - 1].utf8;
        
        for (u16 j = 0; j < m_classFile->m_methods[i].attributeCount; j++)
        {
            const auto& attrib = m_classFile->m_methods[i].attributes[j];
            const auto& attrName = m_classFile->m_constantPool[attrib.nameIndex - 1].utf8;
            if (memcmp(attrName.ptr, "Code", 4) == 0) {
                u8* ptr = attrib.info;
                method.maxStack = parseU16BigEndian(ptr);
                method.maxLocals = parseU16BigEndian(ptr);
                method.codeLength = parseU32BigEndian(ptr);
                method.codePtr = ptr;
                ptr += method.codeLength;
            }
        }

        m_methods.emplace(std::string(name.ptr, name.length), method);
    }
}

Class::~Class()
{
    delete[] m_fields;
    delete[] m_interfaces;
    delete[] m_constantPool;
    delete m_classFile;
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
            const auto& name = m_classFile->m_constantPool[m_fields[i].nameIndex].utf8;
            const auto& desc = m_classFile->m_constantPool[m_fields[i].descriptorIndex].utf8;

            m_staticFields.emplace(std::string(name.ptr, name.length), Value{ .integer = 0 });
            /*const char* ptr = desc.utf8.ptr;
            while (*ptr == '[') ptr++;
            if (*ptr == 'L') {
                ptr++;
                m_classPoolRef.loadClass({ ptr, (u16)(desc.utf8.length - (i16)(ptr - desc.utf8.ptr) - 1) });
            }*/
        }
    }
}
