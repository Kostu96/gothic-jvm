#pragma once
#include "common.hpp"

constexpr u32 InvalidClassIndex = -1;

union ConstantPoolEntry
{
    struct {
        u16 symbolicRef1;
        u16 symbolicRef2;
    };
    u32 index;
};
static_assert(sizeof(ConstantPoolEntry) == 4);

struct Field
{
    u16 accessFlags;
    u16 nameIndex;
    u16 descriptorIndex;
};

struct Method
{
    u16 maxStack;
    u16 maxLocals;
    u32 codeLength;
    u8* codePtr;
};

class ClassFile;
class ClassPool;

// Runtime class
class Class
{
public:
    Class(ClassPool& classPool, const char* classFilename) noexcept;
    ~Class();
    //Class(Class&& other) noexcept;

    void prepare();

    const Method& getMethod(const std::string& name) const { return m_methods.find(name)->second; }

    Class(const Class&) = delete;
    Class& operator=(const Class&) = delete;
private:
    ClassPool& m_classPoolRef;
    const ClassFile* m_classFile = nullptr;

    ConstantPoolEntry* m_constantPool = nullptr;
    u16 m_constantPoolSize = 0;
    u32 m_superClassIndex = InvalidClassIndex;
    u32* m_interfaces = nullptr;
    u16 m_interfacesSize = 0;
    Field* m_fields = nullptr;
    u16 m_fieldsSize = 0;

    std::unordered_map<std::string, Value> m_staticFields;
    std::unordered_map<std::string, Method> m_methods;
};
