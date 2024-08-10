#pragma once
#include "types.hpp"

union ConstantPoolEntry
{
    struct {
        u16 symbolicRef1;
        u16 symbolicRef2;
    };
    u32 classPoolIndex;
};
static_assert(sizeof(ConstantPoolEntry) == 4);

class ClassFile;

// Runtime class
class Class
{
public:
    // Derives runtime class from binary representation
    explicit Class(const ClassFile& classFile);
    ~Class();

    Class(const Class&) = delete;
    Class& operator=(const Class&) = delete;
private:
    ConstantPoolEntry* m_constantPool = nullptr;
    u16 m_constantPoolSize;
};
