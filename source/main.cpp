#include "types.hpp"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <functional>

static bool getFileSize(const char* filename, size_t& fileSize)
{
    FILE* file;
    fopen_s(&file, filename, "rb");
    if (!file) {
        fileSize = 0;
        return false;
    }

    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);

    fclose(file);
    return true;
}

static bool readFile(const char* filename, void* buffer, size_t bufferSize)
{
    FILE* file;
    fopen_s(&file, filename, "rb");
    if (!file) {
        return false;
    }

    fread(buffer, 1, bufferSize, file);

    fclose(file);
    return true;
}

static u16 u16FromBigEndian(u8* ptr) { return *ptr << 8 | *(ptr + 1); }
static u32 u32FromBigEndian(u8* ptr) { return *ptr << 24 | *(ptr + 1) << 16 | *(ptr + 2) << 8 | *(ptr + 3); }
static u64 u64FromBigEndian(u8* ptr) {
    u64 valueH = u32FromBigEndian(ptr);
    u64 valueL = u32FromBigEndian(ptr + 4);
    return valueH << 32 | valueL;
}

static const char* accessFlagToString(uint16_t flag)
{
    if ((flag & AccessFlags::PUBLIC) && (flag & AccessFlags::STATIC)) return "public static";
    if ((flag & AccessFlags::PUBLIC) && (flag & AccessFlags::FINAL)) return "public final";
    if ((flag & AccessFlags::PUBLIC)) return "public";
    if ((flag & AccessFlags::PRIVATE) && (flag & AccessFlags::STATIC)) return "private static";

    assert(false);
    return "???";
}

static void printClassInfo(const char* prefix, u16 index, const ClassFile& classFile)
{
    const ConstPoolInfo& classInfo = classFile.constPool[index - 1];
    assert(classInfo.tag == ConstantTag::Class);
    const ConstPoolInfo& utf8Info = classFile.constPool[classInfo.classInfo.nameIndex - 1];
    assert(utf8Info.tag == ConstantTag::Utf8);
    printf("%s - %.*s\n", prefix, utf8Info.utf8Info.length, utf8Info.utf8Info.ptr);
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

static void loadClassFile(ClassFile& classFile, u8* ptr)
{
    classFile.magic = u32FromBigEndian(ptr); ptr += 4;
    classFile.verMinor = u16FromBigEndian(ptr); ptr += 2;
    classFile.verMajor = u16FromBigEndian(ptr); ptr += 2;

    classFile.constPoolCount = u16FromBigEndian(ptr); ptr += 2;
    classFile.constPool = new ConstPoolInfo[classFile.constPoolCount - 1];

    for (u32 i = 0; i < classFile.constPoolCount - 1; i++)
    {
        classFile.constPool[i].tag = (ConstantTag)*ptr++;
        switch (classFile.constPool[i].tag) {
        case ConstantTag::Utf8: {
            classFile.constPool[i].utf8Info.length = u16FromBigEndian(ptr); ptr += 2;
            classFile.constPool[i].utf8Info.ptr = (const char*)ptr;
            ptr += classFile.constPool[i].utf8Info.length;
        } break;
        case ConstantTag::Integer: {
            classFile.constPool[i].integerInfo.value = u32FromBigEndian(ptr); ptr += 4;
        } break;
        case ConstantTag::Long: {
            classFile.constPool[i].longInfo.value = u64FromBigEndian(ptr); ptr += 8;
            i++;
        } break;
        case ConstantTag::Class:
        case ConstantTag::String:
            classFile.constPool[i].generic2.u16Field = u16FromBigEndian(ptr); ptr += 2;
            break;
        case ConstantTag::FieldRef:
        case ConstantTag::MethodRef:
        case ConstantTag::InterfaceMethodRef:
        case ConstantTag::NameAndType:
            classFile.constPool[i].generic22.u16Field1 = u16FromBigEndian(ptr); ptr += 2;
            classFile.constPool[i].generic22.u16Field2 = u16FromBigEndian(ptr); ptr += 2;
            break;
        default:
            assert(false);
        }
    }

    classFile.accessFlags = u16FromBigEndian(ptr); ptr += 2;
    classFile.thisClass = u16FromBigEndian(ptr); ptr += 2;
    classFile.superClass = u16FromBigEndian(ptr); ptr += 2;

    printClassInfo("This name", classFile.thisClass, classFile);
    printClassInfo("Super name", classFile.superClass, classFile);

    uint16_t interfacesCount = u16FromBigEndian(ptr); ptr += 2;
    printf("Interfaces count - %u:\n", interfacesCount);

    uint16_t* interfaces = new uint16_t[interfacesCount];
    for (uint32_t i = 0; i < interfacesCount; i++)
    {
        interfaces[i] = u16FromBigEndian(ptr); ptr += 2;
        printClassInfo("  ", interfaces[i], classFile);
    }

    uint16_t fieldsCount = u16FromBigEndian(ptr); ptr += 2;
    printf("Fields count - %u:\n", fieldsCount);

    FieldAndMethodInfo* fields = new FieldAndMethodInfo[fieldsCount];
    for (uint32_t i = 0; i < fieldsCount; i++)
    {
        fields[i].accessFlags = u16FromBigEndian(ptr); ptr += 2;
        fields[i].nameIndex = u16FromBigEndian(ptr); ptr += 2;
        fields[i].descriptorIndex = u16FromBigEndian(ptr); ptr += 2;
        fields[i].attributeCount = u16FromBigEndian(ptr); ptr += 2;
        fields[i].attributes = fields[i].attributeCount ? new AttributeInfo[fields[i].attributeCount] : nullptr;

        assert(fields[i].attributeCount == 0); // unhandled attributes

        ConstPoolInfo utf8Info1 = classFile.constPool[fields[i].nameIndex - 1];
        assert(utf8Info1.tag == ConstantTag::Utf8);
        ConstPoolInfo utf8Info2 = classFile.constPool[fields[i].descriptorIndex - 1];
        assert(utf8Info2.tag == ConstantTag::Utf8);
        printf("   %s %.*s : ", accessFlagToString(fields[i].accessFlags),
            utf8Info1.utf8Info.length, utf8Info1.utf8Info.ptr);
        parseFieldDescriptor(utf8Info2.utf8Info.ptr, utf8Info2.utf8Info.length);
        printf("\n");
    }

    uint16_t methodsCount = u16FromBigEndian(ptr); ptr += 2;
    printf("Methods count - %u:\n", methodsCount);

    FieldAndMethodInfo* methods = new FieldAndMethodInfo[methodsCount];
    for (uint32_t i = 0; i < methodsCount; i++)
    {
        methods[i].accessFlags = u16FromBigEndian(ptr); ptr += 2;
        methods[i].nameIndex = u16FromBigEndian(ptr); ptr += 2;
        methods[i].descriptorIndex = u16FromBigEndian(ptr); ptr += 2;
        methods[i].attributeCount = u16FromBigEndian(ptr); ptr += 2;
        methods[i].attributes = methods[i].attributeCount ? new AttributeInfo[methods[i].attributeCount] : nullptr;

        ConstPoolInfo utf8Info1 = classFile.constPool[methods[i].nameIndex - 1];
        assert(utf8Info1.tag == ConstantTag::Utf8);
        ConstPoolInfo utf8Info2 = classFile.constPool[methods[i].descriptorIndex - 1];
        assert(utf8Info2.tag == ConstantTag::Utf8);
        printf("   %s %.*s : %.*s\n", accessFlagToString(methods[i].accessFlags),
            utf8Info1.utf8Info.length, utf8Info1.utf8Info.ptr,
            utf8Info2.utf8Info.length, utf8Info2.utf8Info.ptr);

        for (uint32_t j = 0; j < methods[i].attributeCount; j++)
        {
            methods[i].attributes[j].nameIndex = u16FromBigEndian(ptr); ptr += 2;
            methods[i].attributes[j].length = u32FromBigEndian(ptr); ptr += 4;
            methods[i].attributes[j].info = ptr; ptr += methods[i].attributes[j].length;

            /*ConstPoolInfo utf8Info = classFile.constPool[methods[i].attributes[j].nameIndex - 1];
            assert(utf8Info.tag == ConstantTag::Utf8);
            printf("      %.*s\n", utf8Info.utf8Info.length, utf8Info.utf8Info.ptr);*/
        }
    }

    uint16_t attributesCount = u16FromBigEndian(ptr); ptr += 2;
    printf("Attributes count - %u:\n", attributesCount);

    AttributeInfo* attributes = new AttributeInfo[attributesCount];
    for (uint32_t i = 0; i < attributesCount; i++)
    {
        attributes[i].nameIndex = u16FromBigEndian(ptr); ptr += 2;
        attributes[i].length = u32FromBigEndian(ptr); ptr += 4;
        attributes[i].info = ptr; ptr += attributes[i].length;
    }
}

static void unloadClassFile(ClassFile& classFile)
{
    delete[] classFile.attributes;
    for (uint32_t i = 0; i < classFile.methodsCount; i++)
    {
        delete[] classFile.methods[i].attributes;
    }
    delete[] classFile.methods;
    for (uint32_t i = 0; i < classFile.fieldsCount; i++)
    {
        delete[] classFile.fields[i].attributes;
    }
    delete[] classFile.fields;
    delete[] classFile.interfaces;
    delete[] classFile.constPool;
}

int main()
{
    const char* path = "C:\\Users\\Konstanty\\Desktop\\gothic3the_uste7l3z\\HG.class";
    size_t fileSize;
    if (!getFileSize(path, fileSize)) {
        printf("Could not get file size!\n");
        return -1;
    }
    uint8_t* buffer = new uint8_t[fileSize];
    if (!readFile(path, buffer, fileSize)) {
        printf("Could not read file!\n");
        return -1;
    }

    ClassFile classFile;
    loadClassFile(classFile, buffer);

    unloadClassFile(classFile);
    
    delete[] buffer;
    return 0;
}
