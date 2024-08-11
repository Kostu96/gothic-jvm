#include "class_pool.hpp"

#include <cassert>
#include <functional>

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

int main()
{
    ClassPool classPool;
    classPool.addToClassPath("C:/Users/Konstanty/Desktop/gothic3the_uste7l3z"); // TODO(Kostu): temp
    classPool.addToClassPath("C:/Users/Konstanty/Desktop/midp2.0fcs/classes"); // TODO(Kostu): temp
    
    classPool.loadClass("java/lang/Object");
    u32 HGClassIndex =  classPool.loadClass("HG");
    Class& HGClass = classPool.getClass(HGClassIndex);
    HGClass.prepare();

    /*u8* initCode = nullptr;
    for (u32 i = 0; i < hgClassFile.methodsCount; i++)
    {
        const FieldAndMethodInfo& method = hgClassFile.methods[i];
        const ConstPoolInfo& info = hgClassFile.constPool[method.nameIndex - 1];
        if (memcmp(info.utf8Info.ptr, "<init>", std::min(info.utf8Info.length, (u16)6)) == 0) {
            for (u32 j = 0; j < method.attributeCount; j++)
            {
                const ConstPoolInfo& info = hgClassFile.constPool[method.attributes[j].nameIndex - 1];
                if (memcmp(info.utf8Info.ptr, "Code", std::min(info.utf8Info.length, (u16)4)) == 0) {
                    u8* ptr = method.attributes[j].info;
                    ptr += 4;
                    u32 codeLength = u32FromBigEndian(ptr); ptr += 4;
                    initCode = ptr;
                }
            }
        }
    }
    assert(initCode);

    u32 pc = 0;
    bool notReturned = true;
    while (notReturned) {
        switch (initCode[pc++])
        {
        case 0x04: // iconst_1
            printf("iconst_1\n");
            break;

        case 0x59: // dup
            printf("dup\n");
            break;

        case 0x2A: // aload_0
            printf("aload_0\n");
            break;

        case 0xB1: // return
            printf("return\n");
            notReturned = false;
            break;
        case 0xB2: { // getstatic
            u16 index = initCode[pc] << 8 | initCode[pc + 1];
            pc += 2;
            printf("getstatic %u\n", index);
        } break;
        case 0xB3: { // putstatic
            u16 index = initCode[pc] << 8 | initCode[pc + 1];
            pc += 2;
            printf("putstatic %u\n", index);
        } break;

        case 0xB6: { // invokevirtual
            u16 index = initCode[pc] << 8 | initCode[pc + 1];
            pc += 2;
            printf("invokevirtual %u\n", index);
        } break;
        case 0xB7: { // invokespecial
            u16 index = initCode[pc] << 8 | initCode[pc + 1];
            pc += 2;
            printf("invokespecial %u\n", index);
        } break;
        case 0xB8: { // invokestatic
            u16 index = initCode[pc] << 8 | initCode[pc + 1];
            pc += 2;
            printf("invokestatic %u\n", index);
        } break;

        case 0xBB: { // new
            u16 index = initCode[pc] << 8 | initCode[pc + 1];
            pc += 2;
            printf("new %u\n", index);
        } break;

        case 0xC7: { // ifnonnull
            i16 offset = initCode[pc] << 8 | initCode[pc + 1];
            pc += 2;
            printf("ifnonnull %d\n", offset);
        } break;

        default:
            printf("Unknown opcode: %X\n", initCode[pc - 1]);
            assert(false);
        }
    }*/
    
    return 0;
}
