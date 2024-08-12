#include "class_pool.hpp"
#include "class.hpp"
#include "opcodes.hpp"
#include "call_frame.hpp"

#include <cassert>

int main()
{
    ClassPool classPool;
    classPool.addToClassPath("../../../misc/gothic3thebeginning"); // TODO(Kostu): temp
    classPool.addToClassPath("../../../misc/classes"); // TODO(Kostu): temp
    classPool.addToClassPath("../../../misc/tests"); // TODO(Kostu): temp
    
    classPool.loadClass("java/lang/Object");
    /*u32 HGClassIndex =  classPool.loadClass("HG");
    Class& HGClass = classPool.getClass(HGClassIndex);
    HGClass.prepare();*/

    u32 HelloWorldClassIndex = classPool.loadClass("HelloWorld");
    Class& HelloWorldClass = classPool.getClass(HelloWorldClassIndex);
    HelloWorldClass.prepare();

    struct Instance {
        Class& classRef;
    };

    Instance helloWorldInsntance{ HelloWorldClass };

    std::vector<CallFrame> m_callStack;

    const Method& mainMethod = HelloWorldClass.getMethod("<init>");

    CallFrame callFrame;
    callFrame.pushLocal({ .reference = &helloWorldInsntance });

    m_callStack.push_back(callFrame);

    u32 pc = 0;
    bool notReturned = true;
    while (notReturned) {
        switch (mainMethod.codePtr[pc++])
        {
        case OpCode::iconst_1:
            printf("iconst_1\n");
            break;

        case OpCode::ldc: {
            u8 index = mainMethod.codePtr[pc++];
            printf("ldc %u\n", index);
        } break;

        case OpCode::dup:
            printf("dup\n");
            break;

        case OpCode::aload_0:
            m_callStack.back().aload(0);
            break;

        case OpCode::return_:
            printf("return\n");
            notReturned = false;
            break;
        case OpCode::getstatic: {
            u16 index = mainMethod.codePtr[pc] << 8 | mainMethod.codePtr[pc + 1];
            pc += 2;
            printf("getstatic %u\n", index);
        } break;
        case OpCode::putstatic: {
            u16 index = mainMethod.codePtr[pc] << 8 | mainMethod.codePtr[pc + 1];
            pc += 2;
            printf("putstatic %u\n", index);
        } break;

        case OpCode::invokevirtual: {
            u16 index = mainMethod.codePtr[pc] << 8 | mainMethod.codePtr[pc + 1];
            pc += 2;
            printf("invokevirtual %u\n", index);
        } break;
        case OpCode::invokespecial: {
            u16 index = mainMethod.codePtr[pc] << 8 | mainMethod.codePtr[pc + 1];
            pc += 2;
            printf("invokespecial %u\n", index);
        } break;
        case OpCode::invokestatic: {
            u16 index = mainMethod.codePtr[pc] << 8 | mainMethod.codePtr[pc + 1];
            pc += 2;
            printf("invokestatic %u\n", index);
        } break;

        case OpCode::new_: {
            u16 index = mainMethod.codePtr[pc] << 8 | mainMethod.codePtr[pc + 1];
            pc += 2;
            printf("new %u\n", index);
        } break;

        case OpCode::ifnonnull: {
            i16 offset = mainMethod.codePtr[pc] << 8 | mainMethod.codePtr[pc + 1];
            pc += 2;
            printf("ifnonnull %d\n", offset);
        } break;

        default:
            printf("Unknown opcode: 0x%X\n", mainMethod.codePtr[pc - 1]);
            assert(false);
        }
    }
    
    return 0;
}
