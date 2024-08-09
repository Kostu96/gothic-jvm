#include "types.hpp"
#include "file_io.hpp"
#include "class.hpp"
#include "class_file.hpp"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <functional>
#include <unordered_map>
#include <string>

//static const char* accessFlagToString(uint16_t flag)
//{
//    if ((flag & AccessFlags::PUBLIC) && (flag & AccessFlags::STATIC)) return "public static";
//    if ((flag & AccessFlags::PUBLIC) && (flag & AccessFlags::FINAL)) return "public final";
//    if ((flag & AccessFlags::PUBLIC)) return "public";
//    if ((flag & AccessFlags::PROTECTED)) return "protected";
//    if ((flag & AccessFlags::PRIVATE) && (flag & AccessFlags::STATIC)) return "private static";
//    if ((flag & AccessFlags::PRIVATE)) return "private";
//
//    if (flag == 0) return "";
//
//    assert(false);
//    return "???";
//}

//static void printClassInfo(const char* prefix, u16 index, const ClassFile& classFile)
//{
//    const ConstPoolInfo& classInfo = classFile.constPool[index - 1];
//    assert(classInfo.tag == ConstantTag::Class);
//    const ConstPoolInfo& utf8Info = classFile.constPool[classInfo.classInfo.nameIndex - 1];
//    assert(utf8Info.tag == ConstantTag::Utf8);
//    printf("%s - %.*s\n", prefix, utf8Info.utf8Info.length, utf8Info.utf8Info.ptr);
//}

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

//static void loadClassFile(ClassFile& classFile, u8* ptr)
//{
//    classFile.magic = u32FromBigEndian(ptr); ptr += 4;
//    classFile.verMinor = u16FromBigEndian(ptr); ptr += 2;
//    classFile.verMajor = u16FromBigEndian(ptr); ptr += 2;
//
//    classFile.constPoolCount = u16FromBigEndian(ptr); ptr += 2;
//    classFile.constPool = new ConstPoolInfo[classFile.constPoolCount - 1];
//
//    for (u32 i = 0; i < classFile.constPoolCount - 1; i++)
//    {
//        classFile.constPool[i].tag = (ConstantTag)*ptr++;
//        switch (classFile.constPool[i].tag) {
//        case ConstantTag::Utf8: {
//            classFile.constPool[i].utf8Info.length = u16FromBigEndian(ptr); ptr += 2;
//            classFile.constPool[i].utf8Info.ptr = (const char*)ptr;
//            ptr += classFile.constPool[i].utf8Info.length;
//        } break;
//        case ConstantTag::Integer: {
//            classFile.constPool[i].integerInfo.value = u32FromBigEndian(ptr); ptr += 4;
//        } break;
//        case ConstantTag::Long: {
//            classFile.constPool[i].longInfo.value = u64FromBigEndian(ptr); ptr += 8;
//            i++;
//        } break;
//        case ConstantTag::Class:
//        case ConstantTag::String:
//            classFile.constPool[i].generic2.u16Field = u16FromBigEndian(ptr); ptr += 2;
//            break;
//        case ConstantTag::FieldRef:
//        case ConstantTag::MethodRef:
//        case ConstantTag::InterfaceMethodRef:
//        case ConstantTag::NameAndType:
//            classFile.constPool[i].generic22.u16Field1 = u16FromBigEndian(ptr); ptr += 2;
//            classFile.constPool[i].generic22.u16Field2 = u16FromBigEndian(ptr); ptr += 2;
//            break;
//        default:
//            assert(false);
//        }
//    }
//
//    classFile.accessFlags = u16FromBigEndian(ptr); ptr += 2;
//    classFile.thisClass = u16FromBigEndian(ptr); ptr += 2;
//    classFile.superClass = u16FromBigEndian(ptr); ptr += 2;
//
//    printClassInfo("This name", classFile.thisClass, classFile);
//    printClassInfo("Super name", classFile.superClass, classFile);
//
//    classFile.interfacesCount = u16FromBigEndian(ptr); ptr += 2;
//    printf("Interfaces count - %u:\n", classFile.interfacesCount);
//
//    classFile.interfaces = new u16[classFile.interfacesCount];
//    for (u32 i = 0; i < classFile.interfacesCount; i++)
//    {
//        classFile.interfaces[i] = u16FromBigEndian(ptr); ptr += 2;
//        printClassInfo("  ", classFile.interfaces[i], classFile);
//    }
//
//    classFile.fieldsCount = u16FromBigEndian(ptr); ptr += 2;
//    printf("Fields count - %u:\n", classFile.fieldsCount);
//
//    classFile.fields = new FieldAndMethodInfo[classFile.fieldsCount];
//    for (uint32_t i = 0; i < classFile.fieldsCount; i++)
//    {
//        classFile.fields[i].accessFlags = u16FromBigEndian(ptr); ptr += 2;
//        classFile.fields[i].nameIndex = u16FromBigEndian(ptr); ptr += 2;
//        classFile.fields[i].descriptorIndex = u16FromBigEndian(ptr); ptr += 2;
//        classFile.fields[i].attributeCount = u16FromBigEndian(ptr); ptr += 2;
//        classFile.fields[i].attributes = classFile.fields[i].attributeCount ? new AttributeInfo[classFile.fields[i].attributeCount] : nullptr;
//
//        assert(classFile.fields[i].attributeCount == 0); // unhandled attributes
//
//        ConstPoolInfo utf8Info1 = classFile.constPool[classFile.fields[i].nameIndex - 1];
//        assert(utf8Info1.tag == ConstantTag::Utf8);
//        ConstPoolInfo utf8Info2 = classFile.constPool[classFile.fields[i].descriptorIndex - 1];
//        assert(utf8Info2.tag == ConstantTag::Utf8);
//        printf("   %s %.*s : ", accessFlagToString(classFile.fields[i].accessFlags),
//            utf8Info1.utf8Info.length, utf8Info1.utf8Info.ptr);
//        parseFieldDescriptor(utf8Info2.utf8Info.ptr, utf8Info2.utf8Info.length);
//        printf("\n");
//    }
//
//    classFile.methodsCount = u16FromBigEndian(ptr); ptr += 2;
//    printf("Methods count - %u:\n", classFile.methodsCount);
//
//    classFile.methods = new FieldAndMethodInfo[classFile.methodsCount];
//    for (uint32_t i = 0; i < classFile.methodsCount; i++)
//    {
//        classFile.methods[i].accessFlags = u16FromBigEndian(ptr); ptr += 2;
//        classFile.methods[i].nameIndex = u16FromBigEndian(ptr); ptr += 2;
//        classFile.methods[i].descriptorIndex = u16FromBigEndian(ptr); ptr += 2;
//        classFile.methods[i].attributeCount = u16FromBigEndian(ptr); ptr += 2;
//        classFile.methods[i].attributes = classFile.methods[i].attributeCount ? new AttributeInfo[classFile.methods[i].attributeCount] : nullptr;
//
//        ConstPoolInfo utf8Info1 = classFile.constPool[classFile.methods[i].nameIndex - 1];
//        assert(utf8Info1.tag == ConstantTag::Utf8);
//        ConstPoolInfo utf8Info2 = classFile.constPool[classFile.methods[i].descriptorIndex - 1];
//        assert(utf8Info2.tag == ConstantTag::Utf8);
//        printf("   %s %.*s : %.*s\n", accessFlagToString(classFile.methods[i].accessFlags),
//            utf8Info1.utf8Info.length, utf8Info1.utf8Info.ptr,
//            utf8Info2.utf8Info.length, utf8Info2.utf8Info.ptr);
//
//        for (uint32_t j = 0; j < classFile.methods[i].attributeCount; j++)
//        {
//            classFile.methods[i].attributes[j].nameIndex = u16FromBigEndian(ptr); ptr += 2;
//            classFile.methods[i].attributes[j].length = u32FromBigEndian(ptr); ptr += 4;
//            classFile.methods[i].attributes[j].info = ptr; ptr += classFile.methods[i].attributes[j].length;
//        }
//    }
//
//    classFile.attributesCount = u16FromBigEndian(ptr); ptr += 2;
//    printf("Attributes count - %u:\n", classFile.attributesCount);
//
//    classFile.attributes = new AttributeInfo[classFile.attributesCount];
//    for (uint32_t i = 0; i < classFile.attributesCount; i++)
//    {
//        classFile.attributes[i].nameIndex = u16FromBigEndian(ptr); ptr += 2;
//        classFile.attributes[i].length = u32FromBigEndian(ptr); ptr += 4;
//        classFile.attributes[i].info = ptr; ptr += classFile.attributes[i].length;
//    }
//}

//static void unloadClassFile(ClassFile& classFile)
//{
//    delete[] classFile.attributes;
//    for (uint32_t i = 0; i < classFile.methodsCount; i++)
//    {
//        delete[] classFile.methods[i].attributes;
//    }
//    delete[] classFile.methods;
//    for (uint32_t i = 0; i < classFile.fieldsCount; i++)
//    {
//        delete[] classFile.fields[i].attributes;
//    }
//    delete[] classFile.fields;
//    delete[] classFile.interfaces;
//    delete[] classFile.constPool;
//}

int main()
{
    std::unordered_map<std::string, Class> classPool;

    const char* stringPath = "C:\\Users\\Konstanty\\Desktop\\midp2.0fcs\\classes\\java\\lang\\String.class";
    const char* hgPath = "C:\\Users\\Konstanty\\Desktop\\gothic3the_uste7l3z\\HG.class";

    ClassFile objectClassFile("C:\\Users\\Konstanty\\Desktop\\midp2.0fcs\\classes\\java\\lang\\Object.class");
    objectClassFile.print();
    ClassFile booleanClassFile("C:\\Users\\Konstanty\\Desktop\\midp2.0fcs\\classes\\java\\lang\\Boolean.class");
    booleanClassFile.print();

    ClassFile midletClassFile("C:\\Users\\Konstanty\\Desktop\\midp2.0fcs\\classes\\javax\\microedition\\midlet\\MIDlet.class");
    midletClassFile.print();

    classPool.emplace("java/lang/Object", objectClassFile);
    classPool.emplace("java/lang/Boolean", booleanClassFile);

    classPool.emplace("javax/microedition/midlet/MIDlet", midletClassFile);

    //Class HG("C:\\Users\\Konstanty\\Desktop\\gothic3the_uste7l3z\\HG.class");
    //Class MIDlet("C:\\Users\\Konstanty\\Desktop\\midp2.0fcs\\classes\\javax\\microedition\\midlet\\MIDlet.class");

    //u8* initCode = nullptr;
    //for (u32 i = 0; i < hgClassFile.methodsCount; i++)
    //{
    //    const FieldAndMethodInfo& method = hgClassFile.methods[i];
    //    const ConstPoolInfo& info = hgClassFile.constPool[method.nameIndex - 1];
    //    if (memcmp(info.utf8Info.ptr, "<init>", std::min(info.utf8Info.length, (u16)6)) == 0) {
    //        for (u32 j = 0; j < method.attributeCount; j++)
    //        {
    //            const ConstPoolInfo& info = hgClassFile.constPool[method.attributes[j].nameIndex - 1];
    //            if (memcmp(info.utf8Info.ptr, "Code", std::min(info.utf8Info.length, (u16)4)) == 0) {
    //                u8* ptr = method.attributes[j].info;
    //                ptr += 4;
    //                u32 codeLength = u32FromBigEndian(ptr); ptr += 4;
    //                initCode = ptr;
    //            }
    //        }
    //    }
    //}
    //assert(initCode);

    //u32 pc = 0;
    //bool notReturned = true;
    //while (notReturned) {
    //    switch (initCode[pc++])
    //    {
    //    case 0x04: // iconst_1
    //        printf("iconst_1\n");
    //        break;

    //    case 0x59: // dup
    //        printf("dup\n");
    //        break;

    //    case 0x2A: // aload_0
    //        printf("aload_0\n");
    //        break;

    //    case 0xB1: // return
    //        printf("return\n");
    //        notReturned = false;
    //        break;
    //    case 0xB2: { // getstatic
    //        u16 index = initCode[pc] << 8 | initCode[pc + 1];
    //        pc += 2;
    //        printf("getstatic %u\n", index);
    //    } break;
    //    case 0xB3: { // putstatic
    //        u16 index = initCode[pc] << 8 | initCode[pc + 1];
    //        pc += 2;
    //        printf("putstatic %u\n", index);
    //    } break;

    //    case 0xB6: { // invokevirtual
    //        u16 index = initCode[pc] << 8 | initCode[pc + 1];
    //        pc += 2;
    //        printf("invokevirtual %u\n", index);
    //    } break;
    //    case 0xB7: { // invokespecial
    //        u16 index = initCode[pc] << 8 | initCode[pc + 1];
    //        pc += 2;
    //        printf("invokespecial %u\n", index);
    //    } break;
    //    case 0xB8: { // invokestatic
    //        u16 index = initCode[pc] << 8 | initCode[pc + 1];
    //        pc += 2;
    //        printf("invokestatic %u\n", index);
    //    } break;

    //    case 0xBB: { // new
    //        u16 index = initCode[pc] << 8 | initCode[pc + 1];
    //        pc += 2;
    //        printf("new %u\n", index);
    //    } break;

    //    case 0xC7: { // ifnonnull
    //        i16 offset = initCode[pc] << 8 | initCode[pc + 1];
    //        pc += 2;
    //        printf("ifnonnull %d\n", offset);
    //    } break;

    //    default:
    //        printf("Unknown opcode: %X\n", initCode[pc - 1]);
    //        assert(false);
    //    }
    //}
    
    return 0;
}
