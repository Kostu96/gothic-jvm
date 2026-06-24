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
    const auto code = frame.method()->code;

    while (frame.pc() < code.size()) {
        const auto opcode = std::to_integer<uint8_t>(code[frame.pc()]);
        frame.set_pc(frame.pc() + 1);

        switch (opcode) {
        case 0xB1: // return
            return std::nullopt;

        // TODO: add opcode implementations here. Each case should:
        //   1. Read any inline operands and advance frame.pc() past them.
        //   2. Manipulate frame.operand_stack() / frame.locals().
        //   3. For invocations / static field access, call back into vm_
        //      to load+initialize the target class.

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
