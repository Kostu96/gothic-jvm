#include "runtime/interpreter.hpp"

#include "runtime/class.hpp"
#include "runtime/frame.hpp"
#include "runtime/heap.hpp"
#include "runtime/object.hpp"
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
            auto value = std::get<int32_t>(frame.pop_stack());
            auto index = std::get<int32_t>(frame.pop_stack());
            auto* reference = std::get<Object*>(frame.pop_stack());
            if (reference == nullptr || reference->kind() != Object::Kind::Array) {
                // In a complete VM a null reference would raise NullPointerException.
                throw std::runtime_error("castore: target is not an array reference");
            }
            auto* array = static_cast<ArrayObject*>(reference);
            const auto element = static_cast<int32_t>(static_cast<uint16_t>(value)); // char is 16-bit
            array->set(index, element);
        } break;
        case 0x59: { // dup
            frame.push_stack(frame.peek_stack());
        } break;
        case 0xB1: // return
            return std::nullopt;
        case 0xB3: { // putstatic
            const auto high = std::to_integer<uint16_t>(frame.pop_code_byte());
            const auto low = std::to_integer<uint16_t>(frame.pop_code_byte());
            const auto index = static_cast<uint16_t>((high << 8) | low);

            const FieldAndMethodRef field_ref = frame.owner()->resolve_field_ref(index);

            Class* target = (field_ref.class_name == frame.owner()->this_name())
                ? frame.owner()
                : vm_.load_class(field_ref.class_name);
            vm_.initialize_class(target);

            Value* slot = target->find_static_field(field_ref.name, field_ref.descriptor);
            if (slot == nullptr) {
                // In a complete VM this would raise NoSuchFieldError.
                throw std::runtime_error(std::format(
                    "putstatic: no static field {}.{}:{}",
                    field_ref.class_name, field_ref.name, field_ref.descriptor));
            }

            *slot = frame.pop_stack();
        } break;
        case 0xB7: { // invokespecial
            const auto high = std::to_integer<uint16_t>(frame.pop_code_byte());
            const auto low = std::to_integer<uint16_t>(frame.pop_code_byte());
            const auto index = static_cast<uint16_t>((high << 8) | low);

            const FieldAndMethodRef method_ref = frame.owner()->resolve_field_ref(index);
        } break;
        case 0xBB: { // new
            const auto high = std::to_integer<uint16_t>(frame.pop_code_byte());
            const auto low = std::to_integer<uint16_t>(frame.pop_code_byte());
            const auto index = static_cast<uint16_t>((high << 8) | low);

            const std::string_view class_name = frame.owner()->resolve_class_name(index);

            Class* target = (class_name == frame.owner()->this_name())
                ? frame.owner()
                : vm_.load_class(class_name);
            vm_.initialize_class(target);

            InstanceObject* instance = vm_.heap().new_instance(target);
            frame.push_stack(static_cast<Object*>(instance));
        } break;
        case 0xBC: { // newarray
            auto count = std::get<int32_t>(frame.pop_stack());
            auto type = std::to_integer<uint8_t>(frame.pop_code_byte());
            ArrayObject* array = vm_.heap().new_array(static_cast<ArrayType>(type), count);
            frame.push_stack(static_cast<Object*>(array));
        } break;
        default:
            throw std::runtime_error(std::format(
                "Interpreter: unimplemented opcode 0x{:02X} at pc={} in {}.{}{}",
                opcode, frame.pc() - 1,
                frame.owner()->this_name(), frame.method()->name, frame.method()->descriptor));
        }
    }

    throw std::runtime_error(std::format(
        "Interpreter: fell off the end of {}.{}{}",
        frame.owner()->this_name(), frame.method()->name, frame.method()->descriptor));
}
