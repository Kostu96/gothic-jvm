#include "class_file.hpp"
#include "file_io.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>

ClassFile::ClassFile(const char* path)
{
    m_filename = path;

    size_t fileSize;
    if (!getFileSize(path, fileSize)) {
        printf("Could not get file size of %s\n", path);
        abort();
    }
    m_rawFileBuffer = new u8[fileSize];
    if (!readFile(path, m_rawFileBuffer, fileSize)) {
        printf("Could not read file %s\n", path);
        abort();
    }

    u8* ptr = m_rawFileBuffer;

    m_magic = parseU32BigEndian(ptr);
    m_verMinor = parseU16BigEndian(ptr);
    m_verMajor = parseU16BigEndian(ptr);

    m_constantPoolCount = parseU16BigEndian(ptr);
    m_constantPool = new ConstPoolInfo[m_constantPoolCount - 1];

    for (u16 i = 0; i < m_constantPoolCount - 1; i++)
    {
        m_constantPool[i].tag = (ConstPoolInfo::Tag)parseU8(ptr);
        switch (m_constantPool[i].tag) {
        case ConstPoolInfo::Tag::Utf8: {
            m_constantPool[i].utf8.length = parseU16BigEndian(ptr);
            m_constantPool[i].utf8.ptr = (const char*)ptr;
            ptr += m_constantPool[i].utf8.length;
        } break;
        case ConstPoolInfo::Tag::Integer: {
            m_constantPool[i].integer = parseU32BigEndian(ptr);
        } break;
        case ConstPoolInfo::Tag::Long: {
            m_constantPool[i].longInteger = parseU64BigEndian(ptr);
            i++;
        } break;
        case ConstPoolInfo::Tag::Class:
        case ConstPoolInfo::Tag::String: {
            m_constantPool[i].u16Index = parseU16BigEndian(ptr);
        } break;
        case ConstPoolInfo::Tag::FieldRef:
        case ConstPoolInfo::Tag::MethodRef:
        case ConstPoolInfo::Tag::InterfaceMethodRef:
        case ConstPoolInfo::Tag::NameAndType: {
            m_constantPool[i].doubleIndex.index1 = parseU16BigEndian(ptr);
            m_constantPool[i].doubleIndex.index2 = parseU16BigEndian(ptr);
        } break;
        default:
            assert(false);
        }
    }

    m_accessFlags = parseU16BigEndian(ptr);
    m_thisClass = parseU16BigEndian(ptr);
    m_superClass = parseU16BigEndian(ptr);

    m_interfacesCount = parseU16BigEndian(ptr);
    m_interfaces = new u16[m_interfacesCount];
    for (u16 i = 0; i < m_interfacesCount; i++)
    {
        m_interfaces[i] = parseU16BigEndian(ptr);
    }

    m_fieldsCount = parseU16BigEndian(ptr);
    m_fields = new FieldAndMethodInfo[m_fieldsCount];
    for (u16 i = 0; i < m_fieldsCount; i++)
    {
        m_fields[i].accessFlags = parseU16BigEndian(ptr);
        m_fields[i].nameIndex = parseU16BigEndian(ptr);
        m_fields[i].descriptorIndex = parseU16BigEndian(ptr);
        m_fields[i].attributeCount = parseU16BigEndian(ptr);
        m_fields[i].attributes = m_fields[i].attributeCount ? new AttributeInfo[m_fields[i].attributeCount] : nullptr;

        for (u16 j = 0; j < m_fields[i].attributeCount; j++)
        {
            m_fields[i].attributes[j].nameIndex = parseU16BigEndian(ptr);
            m_fields[i].attributes[j].length = parseU32BigEndian(ptr);
            m_fields[i].attributes[j].info = ptr; ptr += m_fields[i].attributes[j].length;
        }
    }

    m_methodsCount = parseU16BigEndian(ptr);
    m_methods = new FieldAndMethodInfo[m_methodsCount];
    for (u16 i = 0; i < m_methodsCount; i++)
    {
        m_methods[i].accessFlags = parseU16BigEndian(ptr); 
        m_methods[i].nameIndex = parseU16BigEndian(ptr);
        m_methods[i].descriptorIndex = parseU16BigEndian(ptr);
        m_methods[i].attributeCount = parseU16BigEndian(ptr);
        m_methods[i].attributes = m_methods[i].attributeCount ? new AttributeInfo[m_methods[i].attributeCount] : nullptr;

        for (u16 j = 0; j < m_methods[i].attributeCount; j++)
        {
            m_methods[i].attributes[j].nameIndex = parseU16BigEndian(ptr);
            m_methods[i].attributes[j].length = parseU32BigEndian(ptr);
            m_methods[i].attributes[j].info = ptr; ptr += m_methods[i].attributes[j].length;
        }
    }

    m_attributesCount = parseU16BigEndian(ptr);
    m_attributes = new AttributeInfo[m_attributesCount];
    for (u16 i = 0; i < m_attributesCount; i++)
    {
        m_attributes[i].nameIndex = parseU16BigEndian(ptr);
        m_attributes[i].length = parseU32BigEndian(ptr);
        m_attributes[i].info = ptr; ptr += m_attributes[i].length;
    }
}

ClassFile::~ClassFile()
{
    delete[] m_attributes;
    for (u16 i = 0; i < m_methodsCount; i++)
        delete[] m_methods[i].attributes;
    delete[] m_methods;
    for (u16 i = 0; i < m_fieldsCount; i++)
        delete[] m_fields[i].attributes;
    delete[] m_fields;
    delete[] m_interfaces;
    delete[] m_constantPool;
    delete[] m_rawFileBuffer;
}

std::string_view ClassFile::getClassName(u16 index) const
{
    auto& classInfo = m_constantPool[index - 1];
    assert(classInfo.tag == ConstPoolInfo::Tag::Class);
    auto& utf8Info = m_constantPool[classInfo.u16Index - 1];
    assert(utf8Info.tag == ConstPoolInfo::Tag::Utf8);

    return { utf8Info.utf8.ptr, utf8Info.utf8.length };
}

void ClassFile::print() const
{
    printf("ClassFile: %s\n", m_filename);
    printf(" magic: 0x%X\n", m_magic);
    printf(" version: %u.%u\n", m_verMajor, m_verMinor);
    printf(" constant_pool_count: %u\n", m_constantPoolCount);
    if (m_constantPoolCount - 1) {
        printf(" constant_pool:\n");
        printf("  idx %-12s value\n", "type");
    }
    for (u16 i = 0; i < m_constantPoolCount - 1; i++)
    {
        printf("  %2u: ", i + 1);
        switch (m_constantPool[i].tag) {
        case ConstPoolInfo::Tag::Utf8: 
            printf("%-12s %.*s\n", "Utf8", m_constantPool[i].utf8.length, m_constantPool[i].utf8.ptr);
            break;
        case ConstPoolInfo::Tag::Integer:
            printf("%-12s %d\n", "Integer", m_constantPool[i].integer);
            break;
        case ConstPoolInfo::Tag::Long:
            printf("%-12s %lld\n", "Long", m_constantPool[i].longInteger);
            i++;
            break;
        case ConstPoolInfo::Tag::Class:
            printf("%-12s %u\n", "Class", m_constantPool[i].u16Index);
            break;
        case ConstPoolInfo::Tag::String:
            printf("%-12s %u\n", "String", m_constantPool[i].u16Index);
            break;
        case ConstPoolInfo::Tag::FieldRef:
            printf("%-12s %2u %2u\n", "FieldRef", m_constantPool[i].doubleIndex.index1, m_constantPool[i].doubleIndex.index2);
            break;
        case ConstPoolInfo::Tag::MethodRef:
            printf("%-12s %2u %2u\n", "MethodRef", m_constantPool[i].doubleIndex.index1, m_constantPool[i].doubleIndex.index2);
            break;
        case ConstPoolInfo::Tag::InterfaceMethodRef:
            printf("%-12s %2u %2u\n", "InterfaceMethodRef", m_constantPool[i].doubleIndex.index1, m_constantPool[i].doubleIndex.index2);
            break;
        case ConstPoolInfo::Tag::NameAndType:
            printf("%-12s %2u %2u\n", "NameAndType", m_constantPool[i].doubleIndex.index1, m_constantPool[i].doubleIndex.index2);
            break;
        default:
            assert(false);
        }
    }
    printf(" access_flags: 0x%X\n", m_accessFlags);
    printf(" this_class: %u\n", m_thisClass);
    printf(" super_class: %u\n", m_superClass);
    printf(" interfaces_count: %u\n", m_interfacesCount);
    if (m_interfacesCount) {
        printf(" interfaces:\n");
        printf("  idx value\n");
    }
    for (u16 i = 0; i < m_interfacesCount; i++)
    {
        printf("  %2u: %u\n", i, m_interfaces[i]);
    }
    printf(" fields_count: %u\n", m_fieldsCount);
    if (m_fieldsCount) {
        printf(" fields:\n");
        printf("  idx name desc attribs\n");
    }
    for (u16 i = 0; i < m_fieldsCount; i++)
    {
        printf("  %2u: %u   %u   %u\n", i, m_fields[i].nameIndex, m_fields[i].descriptorIndex, m_fields[i].attributeCount);
    }
    printf(" methods_count: %u\n", m_methodsCount);
    if (m_methodsCount) {
        printf(" methods:\n");
        printf("  idx name desc attribs\n");
    }
    for (u16 i = 0; i < m_methodsCount; i++)
    {
        printf("  %2u: %u   %u   %u\n", i, m_methods[i].nameIndex, m_methods[i].descriptorIndex, m_methods[i].attributeCount);
    }
    printf(" attributes_count: %u\n", m_attributesCount);
    if (m_attributesCount) {
        printf(" attributes:\n");
        printf("  idx value\n");
    }
    for (u16 i = 0; i < m_attributesCount; i++)
    {
        printf("  %2u: %u\n", i, m_attributes[i].nameIndex);
    }
}
