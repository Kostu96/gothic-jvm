#pragma once
#include "types.hpp"

constexpr u32 InvalidClassIndex = -1;

union ConstantPoolEntry
{
    struct {
        u16 symbolicRef1;
        u16 symbolicRef2;
    };
    u32 classPoolIndex;
};
static_assert(sizeof(ConstantPoolEntry) == 4);

struct Field
{
    u16 accessFlags;
};

class ClassFile;
class ClassPool;

// Runtime class
class Class
{
public:
    explicit Class(ClassPool& classPool) noexcept : m_classPoolRef{ classPool } {}
    ~Class();
    Class(Class&& other) noexcept;

    // TODO(Kostu): move this to contructor
    void derive(const ClassFile& classFile);
    void prepare();

    Class(const Class&) = delete;
    Class& operator=(const Class&) = delete;
private:
    ClassPool& m_classPoolRef;
    ConstantPoolEntry* m_constantPool = nullptr;
    u16 m_constantPoolSize = 0;
    u32 m_superClassIndex = InvalidClassIndex;
    u32* m_interfaces = nullptr;
    u16 m_interfacesSize = 0;
    Field* m_fields = nullptr;
    u16 m_fieldsSize = 0;
};
