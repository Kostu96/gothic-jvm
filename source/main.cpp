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
    u32 HGClassIndex =  classPool.loadClass("HG");
    Class& HGClass = classPool.getClass(HGClassIndex);
    HGClass.prepare();

    /*u32 HelloWorldClassIndex = classPool.loadClass("HelloWorld");
    Class& HelloWorldClass = classPool.getClass(HelloWorldClassIndex);
    HelloWorldClass.prepare();*/

    struct Instance {
        Class& classRef;
    };

    Instance helloWorldInsntance{ HGClass };

    std::vector<CallFrame> m_callStack;

    const Method& mainMethod = HGClass.getMethod("<init>");

    CallFrame callFrame(HGClass, mainMethod, { { .reference = &helloWorldInsntance } });
    //callFrame.pushLocal({ .reference = &helloWorldInsntance });
    /*callFrame.pushLocal({ .integer = 3 });
    callFrame.pushLocal({ .integer = 5 });*/

    m_callStack.push_back(callFrame);

    bool notReturned = true;
    while (notReturned) {
        switch (m_callStack.back().nextU8())
        {
        case OpCode::iconst_1: printf("iconst_1\n"); break;

        case OpCode::bipush: m_callStack.back().bipush(m_callStack.back().nextU8()); break;

        case OpCode::ldc: {
            u8 index = m_callStack.back().nextU8();
            printf("ldc %u\n", index);
        } break;

        case OpCode::dup:
            printf("dup\n");
            break;

        case OpCode::iadd: m_callStack.back().iadd(); break;

        case OpCode::iload_0: m_callStack.back().iload(0); break;
        case OpCode::iload_1: m_callStack.back().iload(1); break;
        case OpCode::iload_2: m_callStack.back().iload(2); break;
        case OpCode::iload_3: m_callStack.back().iload(3); break;

        case OpCode::aload_0: m_callStack.back().aload(0); break;
        case OpCode::aload_1: m_callStack.back().aload(1); break;
        case OpCode::aload_2: m_callStack.back().aload(2); break;
        case OpCode::aload_3: m_callStack.back().aload(3); break;

        case OpCode::ireturn:
            notReturned = false;
            break;

        case OpCode::return_:
            printf("return\n");
            notReturned = false;
            break;
        case OpCode::getstatic: {
            u16 index = m_callStack.back().nextU16();
            m_callStack.back().getstatic(index);
        } break;
        case OpCode::putstatic: {
            u16 index = m_callStack.back().nextU16();
            m_callStack.back().putstatic(index);
        } break;

        case OpCode::invokevirtual: {
            u16 index = m_callStack.back().nextU16();
            printf("invokevirtual %u\n", index);
        } break;
        case OpCode::invokespecial: {
            u16 index = m_callStack.back().nextU16();
            auto c = m_callStack.back().getCurrentClass().getConstant(index);
            printf("invokespecial %u\n", index);
        } break;
        case OpCode::invokestatic: {
            u16 index = m_callStack.back().nextU16();
            printf("invokestatic %u\n", index);
        } break;

        case OpCode::new_: {
            u16 index = m_callStack.back().nextU16();
            printf("new %u\n", index);
        } break;

        case OpCode::ifnonnull: {
            i16 offset = m_callStack.back().nextU16();
            printf("ifnonnull %d\n", offset);
        } break;

        default:
            printf("Unknown opcode: 0x%X\n", m_callStack.back().prevU8());
            assert(false);
        }
    }
    
    return 0;
}
