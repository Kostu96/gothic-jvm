#include "runtime/interpreter.hpp"

#include "runtime/class.hpp"
#include "runtime/frame.hpp"
#include "runtime/heap.hpp"
#include "runtime/runtime_object.hpp"
#include "runtime/vm.hpp"

#include <algorithm>
#include <format>
#include <print>
#include <stdexcept>
#include <string_view>

namespace {

constexpr int OPCODE_PRINT_PAD_WIDTH = 16;

size_t count_descriptor_arguments(std::string_view descriptor) {
    size_t i = descriptor.find('(');
    if (i == std::string_view::npos) {
        return 0;
    }
    ++i; // skip '('

    size_t count = 0;
    while (i < descriptor.size() && descriptor[i] != ')') {
        switch (descriptor[i]) {
        case '[': // array dimension marker; the component type follows
            ++i;
            continue;
        case 'L': // object reference: L<classname>;
            i = descriptor.find(';', i);
            if (i == std::string_view::npos) {
                return count; // malformed descriptor
            }
            ++i;
            break;
        default: // primitive: B C D F I J S Z
            ++i;
            break;
        }
        ++count;
    }
    return count;
}

const Method* select_virtual_method(VM& vm, Class& start, std::string_view name,
                                    std::string_view descriptor, Class*& owner) {
    for (Class* c = &start; c != nullptr; ) {
        if (const Method* method = c->find_method(name, descriptor)) {
            owner = c;
            return method;
        }
        const std::string_view super = c->super_name();
        c = super.empty() ? nullptr : vm.load_class(super);
    }
    owner = nullptr;
    return nullptr;
}

}

std::optional<Value> Interpreter::execute(Class& owner,
                                          const Method& method,
                                          std::span<const Value> args) {
    std::println("Interpreter: executing {}.{}{} with {} argument(s)",
        owner.this_name(), method.name, method.descriptor, args.size());

    Frame frame(owner, method);

    auto& locals = frame.locals();
    const size_t to_copy = std::min(args.size(), locals.size());
    std::copy_n(args.begin(), to_copy, locals.begin());

    return run(frame);
}

void Interpreter::invoke(Class& owner, const Method& method, size_t arg_count, Frame& frame) {
    if (method.is_native) {
        std::println(
            "Interpreter: executing native {}.{}{}", owner.this_name(), method.name, method.descriptor);
        method.native_callback(vm_, frame);
    }
    else {
        std::vector<Value> args(arg_count);
        for (size_t i = arg_count; i > 0; --i) {
            args[i - 1] = frame.pop_stack();
        }
        execute(owner, method, args);
    }
}

std::optional<Value> Interpreter::run(Frame& frame) {
    while (frame.pc() < frame.method().code.size()) {
        std::print("{:04X}: ", frame.pc());
        const auto opcode = frame.pop_code_u8();

        switch (opcode) {
        case 0x00: { // nop
            std::println("{:{}}", "nop", OPCODE_PRINT_PAD_WIDTH);
        } break;
        case 0x01: { // aconst_null
            std::println("{:{}}", "aconst_null", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<RuntimeObject*>(nullptr));
        } break;
        case 0x02: { // iconst_m1
            std::println("{:{}}", "iconst_m1", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int32_t>(-1));
        } break;
        case 0x03: { // iconst_0
            std::println("{:{}}", "iconst_0", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int32_t>(0));
        } break;
        case 0x04: { // iconst_1
            std::println("{:{}}", "iconst_1", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int32_t>(1));
        } break;
        case 0x05: { // iconst_2
            std::println("{:{}}", "iconst_2", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int32_t>(2));
        } break;
        case 0x06: { // iconst_3
            std::println("{:{}}", "iconst_3", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int32_t>(3));
        } break;
        case 0x07: { // iconst_4
            std::println("{:{}}", "iconst_4", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int32_t>(4));
        } break;
        case 0x08: { // iconst_5
            std::println("{:{}}", "iconst_5", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int32_t>(5));
        } break;
        case 0x09: { // lconst_0
            std::println("{:{}}", "lconst_0", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int64_t>(0));
        } break;
        case 0x0A: { // lconst_1
            std::println("{:{}}", "lconst_1", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int64_t>(1));
        } break;
        case 0x0C: { // fconst_1
            std::println("{:{}}", "fconst_1", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<float>(1.0));
        } break;
        case 0x10: { // bipush
            auto byte = frame.pop_code_u8();
            std::println("{:{}} {:02X}", "bipush", OPCODE_PRINT_PAD_WIDTH, byte);
            frame.push_stack(static_cast<int32_t>(byte));
        } break;
        case 0x11: { // sipush
            auto short_value = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "sipush", OPCODE_PRINT_PAD_WIDTH, short_value);
            frame.push_stack(static_cast<int32_t>(short_value));
        } break;
        case 0x13: { // ldc_w
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "ldc_w", OPCODE_PRINT_PAD_WIDTH, index);
            const auto value = frame.owner().resolve_constant(index);
            frame.push_stack(value);
        } break;
        case 0x14: { // ldc2_w
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "ldc2_w", OPCODE_PRINT_PAD_WIDTH, index);
            const auto value = frame.owner().resolve_constant(index);
            frame.push_stack(value);
        } break;
        case 0x2A: { // aload_0
            std::println("{:{}}", "aload_0", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.locals()[0]);
        } break;
        case 0x2B: { // aload_1
            std::println("{:{}}", "aload_1", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.locals()[1]);
        } break;
        case 0x4B: { // astore_0
            std::println("{:{}}", "astore_0", OPCODE_PRINT_PAD_WIDTH);

            auto value = std::get<RuntimeObject*>(frame.pop_stack());
            frame.locals()[0] = value;
        } break;
        case 0x4C: { // astore_1
            std::println("{:{}}", "astore_1", OPCODE_PRINT_PAD_WIDTH);

            auto value = std::get<RuntimeObject*>(frame.pop_stack());
            frame.locals()[1] = value;
        } break;
        case 0x4F: { // iastore
            std::println("{:{}}", "iastore", OPCODE_PRINT_PAD_WIDTH);

            auto value = std::get<int32_t>(frame.pop_stack());
            auto index = std::get<int32_t>(frame.pop_stack());
            auto* reference = std::get<RuntimeObject*>(frame.pop_stack());
            auto* array = reference ? std::get_if<PrimitiveArrayData>(&reference->data) : nullptr;
            if (array == nullptr) {
                // In a complete VM a null reference would raise NullPointerException.
                throw std::runtime_error("iastore: object is not an array reference");
            }
            array->set(index, value);
        } break;
        case 0x53: { // aastore
            std::println("{:{}}", "aastore", OPCODE_PRINT_PAD_WIDTH);

            auto object = std::get<RuntimeObject*>(frame.pop_stack());
            auto index = std::get<int32_t>(frame.pop_stack());
            auto reference = std::get<RuntimeObject*>(frame.pop_stack());
            auto* array = reference ? std::get_if<InstanceArrayData>(&reference->data) : nullptr;
            if (array == nullptr) {
                // In a complete VM a null reference would raise NullPointerException.
                throw std::runtime_error("aastore: object is not an array reference");
            }
            if (object != nullptr) {
                if (auto* instance = std::get_if<InstanceData>(&object->data)) {
                    if (instance->type != array->element_type) {
                        // In a complete VM this would raise ArrayStoreException.
                        throw std::runtime_error("aastore: object is not an instance of the array's element type");
                    }
                }
            }
            array->set(index, object);
        } break;
        case 0x54: { //bastore
            std::println("{:{}}", "bastore", OPCODE_PRINT_PAD_WIDTH);

            auto value = std::get<int32_t>(frame.pop_stack());
            auto index = std::get<int32_t>(frame.pop_stack());
            auto* reference = std::get<RuntimeObject*>(frame.pop_stack());
            auto* array = reference ? std::get_if<PrimitiveArrayData>(&reference->data) : nullptr;
            if (array == nullptr) {
                // In a complete VM a null reference would raise NullPointerException.
                throw std::runtime_error("bastore: object is not an array reference");
            }
            const auto element = static_cast<int32_t>(static_cast<int8_t>(value)); // byte is 8-bit
            array->set(index, element);
        } break;
        case 0x55: { // castore
            std::println("{:{}}", "castore", OPCODE_PRINT_PAD_WIDTH);

            auto value = std::get<int32_t>(frame.pop_stack());
            auto index = std::get<int32_t>(frame.pop_stack());
            auto* reference = std::get<RuntimeObject*>(frame.pop_stack());
            auto* array = reference ? std::get_if<PrimitiveArrayData>(&reference->data) : nullptr;
            if (array == nullptr) {
                // In a complete VM a null reference would raise NullPointerException.
                throw std::runtime_error("castore: object is not an array reference");
            }
            const auto element = static_cast<int32_t>(static_cast<uint16_t>(value)); // char is 16-bit
            array->set(index, element);
        } break;
        case 0x56: { // sastore
            std::println("{:{}}", "sastore", OPCODE_PRINT_PAD_WIDTH);

            auto value = std::get<int32_t>(frame.pop_stack());
            auto index = std::get<int32_t>(frame.pop_stack());
            auto* reference = std::get<RuntimeObject*>(frame.pop_stack());
            auto* array = reference ? std::get_if<PrimitiveArrayData>(&reference->data) : nullptr;
            if (array == nullptr) {
                // In a complete VM a null reference would raise NullPointerException.
                throw std::runtime_error("sastore: object is not an array reference");
            }
            const auto element = static_cast<int32_t>(static_cast<int16_t>(value)); // short is 16-bit
            array->set(index, element);
        } break;
        case 0x59: { // dup
            std::println("{:{}}", "dup", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.peek_stack());
        } break;
        case 0xB1: { // return
            std::println("{:{}}", "return", OPCODE_PRINT_PAD_WIDTH);
            return std::nullopt;
        }
        case 0xB2: { // getstatic
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "getstatic", OPCODE_PRINT_PAD_WIDTH, index);

            const FieldAndMethodRef field_ref = frame.owner().resolve_field_ref(index);
            Class* target = (field_ref.class_name == frame.owner().this_name())
                ? &frame.owner()
                : vm_.load_class(field_ref.class_name);
            vm_.initialize_class(*target);

            Value* slot = target->find_static_field(field_ref.name, field_ref.descriptor);
            if (slot == nullptr) {
                // In a complete VM this would raise NoSuchFieldError.
                throw std::runtime_error(std::format(
                    "getstatic: no static field {}.{}:{}",
                    field_ref.class_name, field_ref.name, field_ref.descriptor));
            }
            frame.push_stack(*slot);
        } break;
        case 0xB3: { // putstatic
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "putstatic", OPCODE_PRINT_PAD_WIDTH, index);

            const FieldAndMethodRef field_ref = frame.owner().resolve_field_ref(index);
            Class* target = (field_ref.class_name == frame.owner().this_name())
                ? &frame.owner()
                : vm_.load_class(field_ref.class_name);
            vm_.initialize_class(*target);

            Value* slot = target->find_static_field(field_ref.name, field_ref.descriptor);
            if (slot == nullptr) {
                // In a complete VM this would raise NoSuchFieldError.
                throw std::runtime_error(std::format(
                    "putstatic: no static field {}.{}:{}",
                    field_ref.class_name, field_ref.name, field_ref.descriptor));
            }

            *slot = frame.pop_stack();
        } break;
        case 0xB6: { // invokevirtual
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "invokevirtual", OPCODE_PRINT_PAD_WIDTH, index);

            const FieldAndMethodRef method_ref = frame.owner().resolve_method_ref(index);

            // The receiver sits just below the arguments on the operand stack.
            const size_t arg_count = count_descriptor_arguments(method_ref.descriptor);
            const std::vector<Value>& stack = frame.operand_stack();
            auto* receiver = std::get<RuntimeObject*>(stack[stack.size() - 1 - arg_count]);
            if (receiver == nullptr) {
                // In a complete VM this would raise NullPointerException.
                throw std::runtime_error(std::format(
                    "invokevirtual: null receiver for {}.{}{}",
                    method_ref.class_name, method_ref.name, method_ref.descriptor));
            }
            // Resolve the receiver's runtime class for virtual dispatch. Plain
            // instances dispatch against their own class; a java/lang/Class
            // mirror dispatches against java/lang/Class itself.
            Class* receiver_class = nullptr;
            if (auto* instance = std::get_if<InstanceData>(&receiver->data)) {
                receiver_class = instance->type;
            }
            else if (std::holds_alternative<ClassMirrorData>(receiver->data)) {
                receiver_class = vm_.load_class("java/lang/Class");
            }
            else {
                throw std::runtime_error("invokevirtual: receiver is not an instance");
            }

            // Virtual dispatch: select the override starting from the receiver's
            // runtime class and walking up the superclass chain.
            Class& runtime_class = *receiver_class;
            Class* owner = nullptr;
            const Method* method =
                select_virtual_method(vm_, runtime_class, method_ref.name, method_ref.descriptor, owner);
            if (method == nullptr) {
                // In a complete VM this would raise NoSuchMethodError / AbstractMethodError.
                throw std::runtime_error(std::format(
                    "invokevirtual: no method {}.{}{}",
                    method_ref.class_name, method_ref.name, method_ref.descriptor));
            }

            invoke(*owner, *method, arg_count + 1, frame);
        } break;
        case 0xB7: { // invokespecial
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "invokespecial", OPCODE_PRINT_PAD_WIDTH, index);

            const FieldAndMethodRef method_ref = frame.owner().resolve_method_ref(index);
            Class* target = (method_ref.class_name == frame.owner().this_name())
                ? &frame.owner()
                : vm_.load_class(method_ref.class_name);

            const Method* method = target->find_method(method_ref.name, method_ref.descriptor);
            if (method == nullptr) {
                // In a complete VM this would raise NoSuchMethodError.
                throw std::runtime_error(std::format(
                    "invokespecial: no method {}.{}{}",
                    method_ref.class_name, method_ref.name, method_ref.descriptor));
            }

            const size_t slot_count = count_descriptor_arguments(method->descriptor) + 1;
            invoke(*target, *method, slot_count, frame);
        } break;
        case 0xB8: { // invokestatic
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "invokestatic", OPCODE_PRINT_PAD_WIDTH, index);

            const FieldAndMethodRef method_ref = frame.owner().resolve_method_ref(index);
            Class* target = (method_ref.class_name == frame.owner().this_name())
                ? &frame.owner()
                : vm_.load_class(method_ref.class_name);

            const Method* method = target->find_method(method_ref.name, method_ref.descriptor);
            if (method == nullptr) {
                // In a complete VM this would raise NoSuchMethodError.
                throw std::runtime_error(std::format(
                    "invokestatic: no method {}.{}{}",
                    method_ref.class_name, method_ref.name, method_ref.descriptor));
            }

            const size_t slot_count = count_descriptor_arguments(method->descriptor);
            invoke(*target, *method, slot_count, frame);
        } break;
        case 0xBB: { // new
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "new", OPCODE_PRINT_PAD_WIDTH, index);

            const std::string_view class_name = frame.owner().resolve_class_name(index);
            Class* target = (class_name == frame.owner().this_name())
                ? &frame.owner()
                : vm_.load_class(class_name);
            vm_.initialize_class(*target);

            RuntimeObject* instance = vm_.heap().new_instance(*target);
            frame.push_stack(instance);
        } break;
        case 0xBC: { // newarray
            auto type = frame.pop_code_u8();
            std::println("{:{}} {:02X}", "newarray", OPCODE_PRINT_PAD_WIDTH, type);

            auto count = std::get<int32_t>(frame.pop_stack());
            RuntimeObject* array =
                vm_.heap().new_primitive_array(static_cast<ElementType>(type), count);
            frame.push_stack(array);
        } break;
        case 0xBD: { // anewarray
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "anewarray", OPCODE_PRINT_PAD_WIDTH, index);

            auto count = std::get<int32_t>(frame.pop_stack());

            const std::string_view class_name = frame.owner().resolve_class_name(index);
            Class* target = (class_name == frame.owner().this_name())
                ? &frame.owner()
                : vm_.load_class(class_name);
            vm_.initialize_class(*target);

            RuntimeObject* array = vm_.heap().new_instance_array(*target, count);
            frame.push_stack(array);
        } break;
        case 0xBE: { // arraylength
            std::println("{:{}}", "arraylength", OPCODE_PRINT_PAD_WIDTH);

            auto* reference = std::get<RuntimeObject*>(frame.pop_stack());
            if (reference == nullptr) {
                // In a complete VM this would raise NullPointerException.
                throw std::runtime_error("arraylength: object is not an array reference");
            }

            int32_t length = 0;
            if (auto* primitive_array = std::get_if<PrimitiveArrayData>(&reference->data)) {
                length = primitive_array->length();
            }
            else if (auto* instance_array = std::get_if<InstanceArrayData>(&reference->data)) {
                length = instance_array->length();
            }
            else {
                // In a complete VM this would raise ArrayStoreException.
                throw std::runtime_error("arraylength: object is not an array reference");
            }
            frame.push_stack(length);
        } break;
        case 0xC0: { // checkcast
            std::println("{:{}}", "checkcast", OPCODE_PRINT_PAD_WIDTH);
            // NOOP for now
        } break;
        case 0xC5: { // multianewarray
            const auto index = frame.pop_code_u16();
            auto dimensions = frame.pop_code_u8();
            std::println("{:{}} {:04X} {}", "multianewarray", OPCODE_PRINT_PAD_WIDTH, index, dimensions);

            const std::string_view class_name = frame.owner().resolve_class_name(index);
            Class* target = (class_name == frame.owner().this_name())
                ? &frame.owner()
                : vm_.load_class(class_name);
            vm_.initialize_class(*target);

            std::vector<int32_t> counts(dimensions);
            for (int i = dimensions - 1; i >= 0; --i) {
                counts[i] = std::get<int32_t>(frame.pop_stack());
            }
            RuntimeObject* array = vm_.heap().new_instance_array(*target, counts[0]);
            frame.push_stack(array);
        } break;
        case 0xC7: { // ifnonnull
            const auto offset = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "ifnonnull", OPCODE_PRINT_PAD_WIDTH, offset);
            auto* reference = std::get<RuntimeObject*>(frame.pop_stack());
            if (reference != nullptr) {
                frame.set_pc(frame.pc() + offset - 3); // -3 to account for the opcode and offset bytes
            }
        } break;
        default:
            throw std::runtime_error(std::format(
                "Interpreter: unimplemented opcode 0x{:02X} at pc={} in {}.{}{}",
                opcode, frame.pc() - 1,
                frame.owner().this_name(), frame.method().name, frame.method().descriptor));
        }
    }

    throw std::runtime_error(std::format(
        "Interpreter: fell off the end of {}.{}{}",
        frame.owner().this_name(), frame.method().name, frame.method().descriptor));
}
