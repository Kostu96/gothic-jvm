#include "runtime/thread.hpp"

#include "runtime/class.hpp"

#include <print>

void Thread::push_frame(const Method& method, std::span<const Value> args) {
    std::println("Thread: pushing new frame {}.{}{} with {} argument(s)",
        method.owner.this_name(), method.name, method.descriptor, args.size());

    Frame frame(method.owner, method);

    auto& locals = frame.locals();

    // Each argument maps onto locals using its precomputed slot width (long/double
    // occupy two slots); arg_slot_widths already includes the leading `this` slot.
    size_t local_index = 0;
    for (size_t i = 0; i < args.size() && i < method.arg_slot_widths.size(); ++i) {
        if (local_index < locals.size()) {
            locals[local_index] = args[i];
        }
        local_index += method.arg_slot_widths[i];
    }

    frames_.push_back(std::move(frame));
}
