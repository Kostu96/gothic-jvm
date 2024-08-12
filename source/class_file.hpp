#pragma once
#include "common.hpp"

struct ConstPoolInfo {
    enum class Tag : u8 {
        Utf8 = 1,
        Integer = 3,
        Float = 4,
        Long = 5,
        Double = 6,
        Class = 7,
        String = 8,
        FieldRef = 9,
        MethodRef = 10,
        InterfaceMethodRef = 11,
        NameAndType = 12
    };

    union {
        struct {
            const char* ptr;
            u16 length;
        } utf8;
        struct {
            u16 index1;
            u16 index2;
        } doubleIndex;
        u64 longInteger;
        u32 integer;
        u16 u16Index;
    };
    Tag tag;
};

struct AttributeInfo {
    u8* info;
    u32 length;
    u16 nameIndex;
};

struct FieldAndMethodInfo {
    u16 accessFlags;
    u16 nameIndex;
    u16 descriptorIndex;
    u16 attributeCount;
    AttributeInfo* attributes;
};

class Class;

// Binary representation of a class
// https://docs.oracle.com/javase/specs/jvms/se6/html/ClassFile.doc.html
class ClassFile
{
public:
	explicit ClassFile(const char* path);
	~ClassFile();

    std::string_view getClassName(u16 index) const;

	// Print class representation to stdout (for debug)
	void print() const;

    ClassFile(const ClassFile&) = delete;
    ClassFile& operator=(const ClassFile&) = delete;
private:
    const char* m_filename = nullptr;
	u8* m_rawFileBuffer = nullptr;

    u32 m_magic;
    u16 m_verMinor;
    u16 m_verMajor;
    u16 m_constantPoolCount;
    ConstPoolInfo* m_constantPool = nullptr;
    u16 m_accessFlags;
    u16 m_thisClass;
    u16 m_superClass;
    u16 m_interfacesCount;
    u16* m_interfaces = nullptr;
    u16 m_fieldsCount;
    FieldAndMethodInfo* m_fields = nullptr;
    u16 m_methodsCount;
    FieldAndMethodInfo* m_methods = nullptr;
    u16 m_attributesCount;
    AttributeInfo* m_attributes = nullptr;

    friend class Class;
};
