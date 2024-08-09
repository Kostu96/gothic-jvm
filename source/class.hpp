#pragma once
#include "types.hpp"

union ConstantPoolEntry
{

};

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
};
