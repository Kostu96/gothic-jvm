#include "runtime/interpreter.hpp"

#include "runtime/class.hpp"
#include "runtime/frame.hpp"
#include "runtime/vm.hpp"

#include <algorithm>
#include <format>
#include <stdexcept>

std::optional<Value> Interpreter::execute(Class* owner,
                                          const Method& method,
                                          std::span<const Value> args) {
    Frame frame(owner, &method);

    auto& locals = frame.locals();
    const size_t to_copy = std::min(args.size(), locals.size());
    std::copy_n(args.begin(), to_copy, locals.begin());

    return run(frame);
}

std::optional<Value> Interpreter::run(Frame& frame) {
    while (frame.pc() < frame.method()->code.size()) {
        const auto opcode = std::to_integer<uint8_t>(frame.pop_code_byte());

        switch (opcode) {
        case 0x03: { // iconst_0
            frame.push_stack(static_cast<int32_t>(0));
        } break;
        case 0x04: { // iconst_1
            frame.push_stack(static_cast<int32_t>(1));
        } break;
        case 0x05: { // iconst_2
            frame.push_stack(static_cast<int32_t>(2));
        } break;
        case 0x06: { // iconst_3
            frame.push_stack(static_cast<int32_t>(3));
        } break;
        case 0x07: { // iconst_4
            frame.push_stack(static_cast<int32_t>(4));
        } break;
        case 0x08: { // iconst_5
            frame.push_stack(static_cast<int32_t>(5));
        } break;
        case 0x10: { // bipush
            auto byte = std::to_integer<uint8_t>(frame.pop_code_byte());
            frame.push_stack(static_cast<int32_t>(byte));
        } break;
        case 0x55: { // castore
            auto value = frame.pop_stack();
            auto index = frame.pop_stack();
            auto reference = frame.pop_stack();
            // TODO(Kostu96): store into the array under reference
        } break;
        case 0x59: { // dup
            frame.push_stack(frame.peek_stack());
        } break;
        case 0xB1: // return
            return std::nullopt;
        case 0xBC: { // newarray
            auto count = frame.pop_stack();
            auto type = std::to_integer<uint8_t>(frame.pop_code_byte());
            // TODO(Kostu96): create new array on the heap and push reference onto the stack
            frame.push_stack(nullptr); // temp
        } break;
        default:
            throw std::runtime_error(std::format(
                "Interpreter: unimplemented opcode 0x{:02X} at pc={} in {}.{}{}",
                opcode, frame.pc() - 1,
                frame.owner()->name(), frame.method()->name, frame.method()->descriptor));
        }
    }

    throw std::runtime_error(std::format(
        "Interpreter: fell off the end of {}.{}{}",
        frame.owner()->name(), frame.method()->name, frame.method()->descriptor));
}
