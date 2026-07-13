#include "runtime/interpreter.hpp"

#include "runtime/class.hpp"
#include "runtime/frame.hpp"
#include "runtime/heap.hpp"
#include "runtime/object.hpp"
#include "runtime/opcodes.hpp"
#include "runtime/vm.hpp"

#include <algorithm>
#include <format>
#include <print>
#include <stdexcept>
#include <string_view>

namespace {

constexpr int OPCODE_PRINT_PAD_WIDTH = 16;

PrimitiveArrayData::ElementType primitive_array_element_type(std::string_view primitive_name) {
    using enum PrimitiveArrayData::ElementType;
    if (primitive_name == "boolean") return Boolean;
    if (primitive_name == "char")    return Char;
    if (primitive_name == "float")   return Float;
    if (primitive_name == "double")  return Double;
    if (primitive_name == "byte")    return Byte;
    if (primitive_name == "short")   return Short;
    if (primitive_name == "int")     return Int;
    if (primitive_name == "long")    return Long;
    throw std::runtime_error(
        "multianewarray: unsupported primitive component '" + std::string(primitive_name) + "'");
}

}

std::optional<Value> Interpreter::execute(const Method& method,
                                          std::span<const Value> args) {
    std::println("Interpreter: executing {}.{}{} with {} argument(s)",
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

    return run(frame);
}

void Interpreter::invoke(const Method& method, Frame& frame) {
    if (method.is_native) {
        std::println(
            "Interpreter: executing native {}.{}{}", method.owner.this_name(), method.name, method.descriptor);
        if (method.native_callback) {
            (*(method.native_callback))(vm_, frame);
        }
        else {
            throw std::runtime_error(std::format(
                "Failed to call native: {}.{}{}", method.owner.this_name(), method.name, method.descriptor));
        }
    }
    else {
        std::vector<Value> args(method.arg_slot_widths.size());
        for (size_t i = method.arg_slot_widths.size(); i > 0; --i) {
            args[i - 1] = frame.pop_stack();
        }
        auto ret = execute(method, args);
        if (ret.has_value()) {
            frame.push_stack(*ret);
        }
    }
}

// TODO(Kostu): move this to utils
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::optional<Value> Interpreter::run(Frame& frame) {
    while (frame.get_pc() < frame.method().code.size()) {
        if (vm_.stop_requested()) {
            throw VmStopRequested{};
        }
        std::print("Stack: [");
        for (size_t i = 0; i < frame.operand_stack().size(); ++i) {
            if (i > 0) {
                std::print(", ");
            }
            auto val = frame.operand_stack()[i];
            std::visit(overloaded{
                [](std::monostate) { std::print("!invalid!"); },
                [](int32_t arg) { std::print("int: {}", arg); },
                [](int64_t arg) { std::print("long: {}", arg); },
                [](float arg) { std::print("float: {}", arg); },
                [](double arg) { std::print("double: {}", arg); },
                [](Object* arg) { std::print("object: {}", static_cast<const void*>(arg)); }
            }, val);
        }
        std::println("]");
        std::print("{:04X}: ", frame.get_pc());
        const auto opcode = frame.pop_code_u8();

        switch (opcode) {
        case op_nop: {
            std::println("{:{}}", "nop", OPCODE_PRINT_PAD_WIDTH);
        } break;
        case op_aconst_null: {
            std::println("{:{}}", "aconst_null", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<Object*>(nullptr));
        } break;
        case op_iconst_m1: {
            std::println("{:{}}", "iconst_m1", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int32_t>(-1));
        } break;
        case op_iconst_0: {
            std::println("{:{}}", "iconst_0", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int32_t>(0));
        } break;
        case op_iconst_1: { 
            std::println("{:{}}", "iconst_1", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int32_t>(1));
        } break;
        case op_iconst_2: {
            std::println("{:{}}", "iconst_2", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int32_t>(2));
        } break;
        case op_iconst_3: {
            std::println("{:{}}", "iconst_3", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int32_t>(3));
        } break;
        case op_iconst_4: {
            std::println("{:{}}", "iconst_4", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int32_t>(4));
        } break;
        case op_iconst_5: {
            std::println("{:{}}", "iconst_5", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int32_t>(5));
        } break;
        case op_lconst_0: {
            std::println("{:{}}", "lconst_0", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int64_t>(0));
        } break;
        case op_lconst_1: {
            std::println("{:{}}", "lconst_1", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<int64_t>(1));
        } break;
        case op_fconst_0: {
            std::println("{:{}}", "fconst_0", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<float>(0.0));
        } break;
        case op_fconst_1: {
            std::println("{:{}}", "fconst_1", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<float>(1.0));
        } break;
        case op_fconst_2: {
            std::println("{:{}}", "fconst_2", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<float>(2.0));
        } break;
        case op_dconst_0: {
            std::println("{:{}}", "dconst_0", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<double>(0.0));
        } break;
        case op_dconst_1: {
            std::println("{:{}}", "dconst_1", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(static_cast<double>(1.0));
        } break;
        case op_bipush: {
            auto byte = frame.pop_code_u8();
            std::println("{:{}} {:02X}", "bipush", OPCODE_PRINT_PAD_WIDTH, byte);
            frame.push_stack(static_cast<int32_t>(byte));
        } break;
        case op_sipush: {
            auto short_value = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "sipush", OPCODE_PRINT_PAD_WIDTH, short_value);
            frame.push_stack(static_cast<int32_t>(short_value));
        } break;
        case op_ldc: {
            const auto index = frame.pop_code_u8();
            std::println("{:{}} {:02X}", "ldc", OPCODE_PRINT_PAD_WIDTH, index);
            const auto value = frame.owner().resolve_constant(index, vm_.class_loader(), vm_.heap());
            frame.push_stack(value);
        } break;
        case op_ldc_w: {
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "ldc_w", OPCODE_PRINT_PAD_WIDTH, index);
            const auto value = frame.owner().resolve_constant(index, vm_.class_loader(), vm_.heap());
            frame.push_stack(value);
        } break;
        case op_ldc2_w: {
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "ldc2_w", OPCODE_PRINT_PAD_WIDTH, index);
            const auto value = frame.owner().resolve_constant(index, vm_.class_loader(), vm_.heap());
            frame.push_stack(value);
        } break;
        case op_iload: {
            auto index = frame.pop_code_u8();
            std::println("{:{}} {:02X}", "iload", OPCODE_PRINT_PAD_WIDTH, index);
            frame.push_stack(frame.locals()[index]);
        } break;
        
        case op_aload: {
            auto index = frame.pop_code_u8();
            std::println("{:{}} {:02X}", "aload", OPCODE_PRINT_PAD_WIDTH, index);
            frame.push_stack(frame.locals()[index]);
        } break;
        case op_iload_0: {
            std::println("{:{}}", "iload_0", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.locals()[0]);
        } break;
        case op_iload_1: {
            std::println("{:{}}", "iload_1", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.locals()[1]);
        } break;
        case op_iload_2: {
            std::println("{:{}}", "iload_2", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.locals()[2]);
        } break;
        case op_iload_3: {
            std::println("{:{}}", "iload_3", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.locals()[3]);
        } break;
        case op_lload_0: {
            std::println("{:{}}", "lload_0", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.locals()[0]);
        } break;
        case op_lload_1: {
            std::println("{:{}}", "lload_1", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.locals()[1]);
        } break;
        case op_lload_2: {
            std::println("{:{}}", "lload_2", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.locals()[2]);
        } break;
        case op_lload_3: {
            std::println("{:{}}", "lload_3", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.locals()[3]);
        } break;

        case op_aload_0: {
            std::println("{:{}}", "aload_0", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.locals()[0]);
        } break;
        case op_aload_1: {
            std::println("{:{}}", "aload_1", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.locals()[1]);
        } break;
        case op_aload_2: {
            std::println("{:{}}", "aload_2", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.locals()[2]);
        } break;
        case op_aload_3: {
            std::println("{:{}}", "aload_3", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.locals()[3]);
        } break;

        case op_aaload: {
            std::println("{:{}}", "aaload", OPCODE_PRINT_PAD_WIDTH);
            auto index = std::get<int32_t>(frame.pop_stack());
            auto* reference = std::get<Object*>(frame.pop_stack());
            auto* array = reference ? std::get_if<InstanceArrayData>(&reference->data) : nullptr;
            if (array == nullptr) {
                // In a complete VM a null reference would raise NullPointerException.
                throw std::runtime_error("aaload: object is not an array reference");
            }
            frame.push_stack(array->elements[index]);
        } break;

        case op_caload: {
            std::println("{:{}}", "caload", OPCODE_PRINT_PAD_WIDTH);

            auto index = std::get<int32_t>(frame.pop_stack());
            auto* reference = std::get<Object*>(frame.pop_stack());
            auto* array = reference ? std::get_if<PrimitiveArrayData>(&reference->data) : nullptr;
            if (array == nullptr) {
                // In a complete VM a null reference would raise NullPointerException.
                throw std::runtime_error("caload: object is not an array reference");
            }
            frame.push_stack(array->get(index));
        } break;

        case op_istore: {
            auto index = frame.pop_code_u8();
            std::println("{:{}} {:02X}", "istore", OPCODE_PRINT_PAD_WIDTH, index);
            frame.locals()[index] = frame.pop_stack();
        } break;

        case op_astore: {
            auto index = frame.pop_code_u8();
            std::println("{:{}} {:02X}", "astore", OPCODE_PRINT_PAD_WIDTH, index);
            frame.locals()[index] = frame.pop_stack();
        } break;
        case op_istore_0: {
            std::println("{:{}}", "istore_0", OPCODE_PRINT_PAD_WIDTH);
            frame.locals()[0] = frame.pop_stack();
        } break;
        case op_istore_1: {
            std::println("{:{}}", "istore_1", OPCODE_PRINT_PAD_WIDTH);
            frame.locals()[1] = frame.pop_stack();
        } break;
        case op_istore_2: {
            std::println("{:{}}", "istore_2", OPCODE_PRINT_PAD_WIDTH);
            frame.locals()[2] = frame.pop_stack();
        } break;
        case op_istore_3: {
            std::println("{:{}}", "istore_3", OPCODE_PRINT_PAD_WIDTH);
            frame.locals()[3] = frame.pop_stack();
        } break;

        case op_astore_0: {
            std::println("{:{}}", "astore_0", OPCODE_PRINT_PAD_WIDTH);
            auto value = std::get<Object*>(frame.pop_stack());
            frame.locals()[0] = value;
        } break;
        case op_astore_1: {
            std::println("{:{}}", "astore_1", OPCODE_PRINT_PAD_WIDTH);
            auto value = std::get<Object*>(frame.pop_stack());
            frame.locals()[1] = value;
        } break;
        case op_astore_2: {
            std::println("{:{}}", "astore_2", OPCODE_PRINT_PAD_WIDTH);
            auto value = std::get<Object*>(frame.pop_stack());
            frame.locals()[2] = value;
        } break;
        case op_astore_3: {
            std::println("{:{}}", "astore_3", OPCODE_PRINT_PAD_WIDTH);
            auto value = std::get<Object*>(frame.pop_stack());
            frame.locals()[3] = value;
        } break;
        case op_iastore: {
            std::println("{:{}}", "iastore", OPCODE_PRINT_PAD_WIDTH);

            auto value = std::get<int32_t>(frame.pop_stack());
            auto index = std::get<int32_t>(frame.pop_stack());
            auto* reference = std::get<Object*>(frame.pop_stack());
            auto* array = reference ? std::get_if<PrimitiveArrayData>(&reference->data) : nullptr;
            if (array == nullptr) {
                // In a complete VM a null reference would raise NullPointerException.
                throw std::runtime_error("iastore: object is not an array reference");
            }
            array->set(index, value);
        } break;
        
        case op_aastore: {
            std::println("{:{}}", "aastore", OPCODE_PRINT_PAD_WIDTH);

            auto object = std::get<Object*>(frame.pop_stack());
            auto index = std::get<int32_t>(frame.pop_stack());
            auto reference = std::get<Object*>(frame.pop_stack());
            auto* array = reference ? std::get_if<InstanceArrayData>(&reference->data) : nullptr;
            if (array == nullptr) {
                // In a complete VM a null reference would raise NullPointerException.
                throw std::runtime_error("aastore: object is not an array reference");
            }
            if (object != nullptr) {
                if (auto* instance = std::get_if<InstanceData>(&object->data)) {
                    if (&instance->type != &array->element_type) {
                        // In a complete VM this would raise ArrayStoreException.
                        throw std::runtime_error("aastore: object is not an instance of the array's element type");
                    }
                }
            }
            array->elements[index] = object;
        } break;
        case op_bastore: {
            std::println("{:{}}", "bastore", OPCODE_PRINT_PAD_WIDTH);

            auto value = std::get<int32_t>(frame.pop_stack());
            auto index = std::get<int32_t>(frame.pop_stack());
            auto* reference = std::get<Object*>(frame.pop_stack());
            auto* array = reference ? std::get_if<PrimitiveArrayData>(&reference->data) : nullptr;
            if (array == nullptr) {
                // In a complete VM a null reference would raise NullPointerException.
                throw std::runtime_error("bastore: object is not an array reference");
            }
            const auto element = static_cast<int32_t>(static_cast<int8_t>(value)); // byte is 8-bit
            array->set(index, element);
        } break;
        case op_castore: {
            std::println("{:{}}", "castore", OPCODE_PRINT_PAD_WIDTH);

            auto value = std::get<int32_t>(frame.pop_stack());
            auto index = std::get<int32_t>(frame.pop_stack());
            auto* reference = std::get<Object*>(frame.pop_stack());
            auto* array = reference ? std::get_if<PrimitiveArrayData>(&reference->data) : nullptr;
            if (array == nullptr) {
                // In a complete VM a null reference would raise NullPointerException.
                throw std::runtime_error("castore: object is not an array reference");
            }
            const auto element = static_cast<int32_t>(static_cast<uint16_t>(value)); // char is 16-bit
            array->set(index, element);
        } break;
        case op_sastore: {
            std::println("{:{}}", "sastore", OPCODE_PRINT_PAD_WIDTH);

            auto value = std::get<int32_t>(frame.pop_stack());
            auto index = std::get<int32_t>(frame.pop_stack());
            auto* reference = std::get<Object*>(frame.pop_stack());
            auto* array = reference ? std::get_if<PrimitiveArrayData>(&reference->data) : nullptr;
            if (array == nullptr) {
                // In a complete VM a null reference would raise NullPointerException.
                throw std::runtime_error("sastore: object is not an array reference");
            }
            const auto element = static_cast<int32_t>(static_cast<int16_t>(value)); // short is 16-bit
            array->set(index, element);
        } break;
        
        case op_dup: {
            std::println("{:{}}", "dup", OPCODE_PRINT_PAD_WIDTH);
            frame.push_stack(frame.peek_stack());
        } break;

        case op_iadd: {
            std::println("{:{}}", "iadd", OPCODE_PRINT_PAD_WIDTH);
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(value1 + value2);
        } break;

        case op_isub: {
            std::println("{:{}}", "isub", OPCODE_PRINT_PAD_WIDTH);
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(value1 - value2);
        } break;

        case op_imul: {
            std::println("{:{}}", "imul", OPCODE_PRINT_PAD_WIDTH);
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(value1 * value2);
        } break;

        case op_idiv: {
            std::println("{:{}}", "idiv", OPCODE_PRINT_PAD_WIDTH);
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            if (value2 == 0) {
                // In a complete VM this would raise ArithmeticException.
                throw std::runtime_error("idiv: division by zero");
            }
            frame.push_stack(value1 / value2);
        } break;

        case op_ishl: {
            std::println("{:{}}", "ishl", OPCODE_PRINT_PAD_WIDTH);
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(value1 << (value2 & 0x1F));
        } break;

        case op_land: {
            std::println("{:{}}", "land", OPCODE_PRINT_PAD_WIDTH);
            auto value2 = std::get<int64_t>(frame.pop_stack());
            auto value1 = std::get<int64_t>(frame.pop_stack());
            frame.push_stack(value1 & value2);
        } break;
        case op_ior: {
            std::println("{:{}}", "ior", OPCODE_PRINT_PAD_WIDTH);
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(value1 | value2);
        } break;

        case op_lxor: {
            std::println("{:{}}", "lxor", OPCODE_PRINT_PAD_WIDTH);
            auto value2 = std::get<int64_t>(frame.pop_stack());
            auto value1 = std::get<int64_t>(frame.pop_stack());
            frame.push_stack(value1 ^ value2);
        } break;
        case op_iinc: {
            std::println("{:{}}", "iinc", OPCODE_PRINT_PAD_WIDTH);
            auto index = frame.pop_code_u8();
            auto constant = static_cast<int8_t>(frame.pop_code_u8());
            frame.locals()[index] = std::get<int32_t>(frame.locals()[index]) + constant;
        } break;

        case op_i2s: {
            std::println("{:{}}", "i2s", OPCODE_PRINT_PAD_WIDTH);
            auto value = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(static_cast<int32_t>(static_cast<int16_t>(value)));
        } break;

        case op_ifne: {
            const auto offset = static_cast<int16_t>(frame.pop_code_u16());
            std::println("{:{}} {:04X}", "ifne", OPCODE_PRINT_PAD_WIDTH, offset);
            
            auto value = std::get<int32_t>(frame.pop_stack());
            if (value != 0) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_iflt: {
            const auto offset = static_cast<int16_t>(frame.pop_code_u16());
            std::println("{:{}} {:04X}", "iflt", OPCODE_PRINT_PAD_WIDTH, offset);

            auto value = std::get<int32_t>(frame.pop_stack());
            if (value < 0) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_ifge: {
            const auto offset = static_cast<int16_t>(frame.pop_code_u16());
            std::println("{:{}} {:04X}", "ifge", OPCODE_PRINT_PAD_WIDTH, offset);
            
            auto value = std::get<int32_t>(frame.pop_stack());
            if (value >= 0) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;

        case op_ifle: {
            const auto offset = static_cast<int16_t>(frame.pop_code_u16());
            std::println("{:{}} {:04X}", "ifle", OPCODE_PRINT_PAD_WIDTH, offset);
            
            auto value = std::get<int32_t>(frame.pop_stack());
            if (value <= 0) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_if_icmpeq: {
            const auto offset = static_cast<int16_t>(frame.pop_code_u16());
            std::println("{:{}} {:04X}", "if_icmpeq", OPCODE_PRINT_PAD_WIDTH, offset);
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            if (value1 == value2) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_if_icmpne: {
            const auto offset = static_cast<int16_t>(frame.pop_code_u16());
            std::println("{:{}} {:04X}", "if_icmpne", OPCODE_PRINT_PAD_WIDTH, offset);

            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            if (value1 != value2) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_if_icmplt: {
            const auto offset = static_cast<int16_t>(frame.pop_code_u16());
            std::println("{:{}} {:04X}", "if_icmplt", OPCODE_PRINT_PAD_WIDTH, offset);

            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            if (value1 < value2) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_if_icmpge: {
            const auto offset = static_cast<int16_t>(frame.pop_code_u16());
            std::println("{:{}} {:04X}", "if_icmpge", OPCODE_PRINT_PAD_WIDTH, offset);

            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            if (value1 >= value2) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;

        case op_goto: {
            const auto offset = static_cast<int16_t>(frame.pop_code_u16());
            std::println("{:{}} {:04X}", "goto", OPCODE_PRINT_PAD_WIDTH, offset);

            frame.branch(offset - 3); // -3 to account for the size of the instruction itself
        } break;

        case op_ireturn: {
            std::println("{:{}}", "ireturn", OPCODE_PRINT_PAD_WIDTH);
            if (frame.operand_stack().size() != 1) {
                throw std::runtime_error("ireturn: operand stack should have exactly one value");
            }
            return frame.pop_stack();
        }

        case op_areturn: {
            std::println("{:{}}", "areturn", OPCODE_PRINT_PAD_WIDTH);
            if (frame.operand_stack().size() != 1) {
                throw std::runtime_error("areturn: operand stack should have exactly one value");
            }
            return frame.pop_stack();
        }
        case op_return: {
            std::println("{:{}}", "return", OPCODE_PRINT_PAD_WIDTH);
            if (!frame.operand_stack().empty()) {
                throw std::runtime_error("return: operand stack should be empty");
            }
            return std::nullopt;
        }
        case op_getstatic: {
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "getstatic", OPCODE_PRINT_PAD_WIDTH, index);

            auto& field = frame.owner().resolve_field(index, vm_.class_loader());
            if (!field.is_static) {
                throw std::runtime_error(std::format(
                    "getstatic: field {}.{}:{} is not static",
                    field.owner.this_name(), field.name, field.descriptor));
            }

            vm_.initialize_class(field.owner);
            frame.push_stack(field.value);
        } break;
        case op_putstatic: {
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "putstatic", OPCODE_PRINT_PAD_WIDTH, index);

            auto& field = frame.owner().resolve_field(index, vm_.class_loader());
            if (!field.is_static) {
                throw std::runtime_error(std::format(
                    "putstatic: field {}.{}:{} is not static",
                    field.owner.this_name(), field.name, field.descriptor));
            }

            vm_.initialize_class(field.owner);
            field.value = frame.pop_stack();
        } break;
        case op_getfield: {
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "getfield", OPCODE_PRINT_PAD_WIDTH, index);

            auto& field = frame.owner().resolve_field(index, vm_.class_loader());
            if (field.is_static) {
                throw std::runtime_error(std::format(
                    "getfield: field {}.{}:{} is static, expected instance field",
                    field.owner.this_name(), field.name, field.descriptor));
            }

            auto object = std::get<Object*>(frame.pop_stack());
            frame.push_stack(std::get<InstanceData>(object->data).fields[field.slot]);
        } break;
        case op_putfield: {
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "putfield", OPCODE_PRINT_PAD_WIDTH, index);

            auto& field = frame.owner().resolve_field(index, vm_.class_loader());
            if (field.is_static) {
                throw std::runtime_error(std::format(
                    "putfield: field {}.{}:{} is static, expected instance field",
                    field.owner.this_name(), field.name, field.descriptor));
            }

            auto value = frame.pop_stack();
            auto object = std::get<Object*>(frame.pop_stack());
            std::get<InstanceData>(object->data).fields[field.slot] = value;
        } break;
        case op_invokevirtual: {
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "invokevirtual", OPCODE_PRINT_PAD_WIDTH, index);

            const Method& resolved_method = frame.owner().resolve_method(index, vm_.class_loader());

            auto* receiver = std::get<Object*>(frame.peek_stack(resolved_method.arg_slot_widths.size() - 1));
            if (receiver == nullptr) {
                // In a complete VM this would raise NullPointerException.
                throw std::runtime_error(std::format(
                    "invokevirtual: null receiver for {}.{}{}",
                    resolved_method.owner.this_name(), resolved_method.name, resolved_method.descriptor));
            }

            Class* receiver_class = nullptr;
            if (auto* instance = std::get_if<InstanceData>(&receiver->data)) {
                receiver_class = &instance->type;
            }
            else if (std::holds_alternative<ClassMirrorData>(receiver->data)) {
                receiver_class = &vm_.class_loader().load("java/lang/Class");
            }
            else {
                throw std::runtime_error("invokevirtual: receiver is not an instance");
            }

            auto find_method_in_superclasses = [&](Class* cls) -> const Method* {
                while (cls != nullptr) {
                    const Method* method = cls->find_method(resolved_method.name, resolved_method.descriptor);
                    if (method != nullptr) {
                        return method;
                    }
                    cls = cls->super();
                }
                return nullptr;
            };
            const Method* method = find_method_in_superclasses(receiver_class);
            if (method == nullptr) {
                // In a complete VM this would raise NoSuchMethodError / AbstractMethodError.
                throw std::runtime_error(std::format(
                    "invokevirtual: no method {}.{}{}",
                    resolved_method.owner.this_name(), resolved_method.name, resolved_method.descriptor));
            }

            invoke(*method, frame);
        } break;
        case op_invokespecial: {
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "invokespecial", OPCODE_PRINT_PAD_WIDTH, index);

            const Method& method = frame.owner().resolve_method(index, vm_.class_loader());
            bool is_constructor = (method.name == "<init>");
            bool is_interface = method.owner.is_interface();
            bool is_superclass = (&method.owner == frame.owner().super());
            if (!is_constructor && !is_interface && is_superclass && frame.owner().treat_super_specially()) {
                throw std::runtime_error("invokespecial: ACC_SUPER semantics not implemented yet");
            }

            invoke(method, frame);
        } break;
        case op_invokestatic: {
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "invokestatic", OPCODE_PRINT_PAD_WIDTH, index);

            const Method& method = frame.owner().resolve_method(index, vm_.class_loader());
            if (!method.is_static) {
                throw std::runtime_error(std::format(
                    "invokestatic: method {}.{}{} is not static",
                    method.owner.this_name(), method.name, method.descriptor));
            }

            vm_.initialize_class(method.owner);
            invoke(method, frame);
        } break;

        case op_new: {
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "new", OPCODE_PRINT_PAD_WIDTH, index);

            Class& target = frame.owner().resolve_class(index, vm_.class_loader());
            vm_.initialize_class(target);

            Object* instance = vm_.heap().new_instance(target);
            frame.push_stack(instance);
        } break;
        case op_newarray: {
            auto type = frame.pop_code_u8();
            std::println("{:{}} {:02X}", "newarray", OPCODE_PRINT_PAD_WIDTH, type);

            auto count = std::get<int32_t>(frame.pop_stack());
            Object* array =
                vm_.heap().new_primitive_array(static_cast<PrimitiveArrayData::ElementType>(type), count);
            frame.push_stack(array);
        } break;
        case op_anewarray: {
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "anewarray", OPCODE_PRINT_PAD_WIDTH, index);

            Class& target = frame.owner().resolve_class(index, vm_.class_loader());

            auto count = std::get<int32_t>(frame.pop_stack());
            Object* array = vm_.heap().new_instance_array(target, count);
            frame.push_stack(array);
        } break;
        case op_arraylength: {
            std::println("{:{}}", "arraylength", OPCODE_PRINT_PAD_WIDTH);

            auto* reference = std::get<Object*>(frame.pop_stack());
            if (reference == nullptr) {
                // In a complete VM this would raise NullPointerException.
                throw std::runtime_error("arraylength: object is not an array reference");
            }

            int32_t length = 0;
            if (auto* primitive_array = std::get_if<PrimitiveArrayData>(&reference->data)) {
                length = primitive_array->length();
            }
            else if (auto* instance_array = std::get_if<InstanceArrayData>(&reference->data)) {
                length = static_cast<int32_t>(instance_array->elements.size());
            }
            else {
                // In a complete VM this would raise ArrayStoreException.
                throw std::runtime_error("arraylength: object is not an array reference");
            }
            frame.push_stack(length);
        } break;

        case op_checkcast: {
            const auto index = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "checkcast", OPCODE_PRINT_PAD_WIDTH, index);

            Class& target = frame.owner().resolve_class(index, vm_.class_loader());
            // NOOP for now but resolves a class
        } break;

        case op_multianewarray: {
            const auto index = frame.pop_code_u16();
            auto dimensions = frame.pop_code_u8();
            std::println("{:{}} {:04X} {}", "multianewarray", OPCODE_PRINT_PAD_WIDTH, index, dimensions);

            if (dimensions < 1) {
                throw std::runtime_error("multianewarray: dimensions must be at least 1");
            }

            Class& target = frame.owner().resolve_class(index, vm_.class_loader());

            std::vector<int32_t> counts(dimensions);
            for (int i = dimensions - 1; i >= 0; --i) {
                counts[i] = std::get<int32_t>(frame.pop_stack());
            }

            auto allocate =
                [&](this const auto& self, Class& array_class, int level) -> Object* {
                Class& component = *array_class.component_type();

                if (component.kind() == Class::Kind::Primitive) {
                    return vm_.heap().new_primitive_array(
                        primitive_array_element_type(component.this_name()), counts[level]);
                }

                Object* array = vm_.heap().new_instance_array(component, counts[level]);
                if (level + 1 < dimensions) {
                    auto& elements = std::get<InstanceArrayData>(array->data).elements;
                    for (int32_t i = 0; i < counts[level]; ++i) {
                        elements[i] = self(component, level + 1);
                    }
                }

                return array;
            };

            Object* array = allocate(target, 0);
            frame.push_stack(array);
        } break;
        case op_ifnull: {
            const auto offset = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "ifnull", OPCODE_PRINT_PAD_WIDTH, offset);
            auto* reference = std::get<Object*>(frame.pop_stack());
            if (reference == nullptr) {
                frame.branch(offset - 3); // -3 to account for the opcode and offset bytes
            }
        } break;
        case op_ifnonnull: {
            const auto offset = frame.pop_code_u16();
            std::println("{:{}} {:04X}", "ifnonnull", OPCODE_PRINT_PAD_WIDTH, offset);
            auto* reference = std::get<Object*>(frame.pop_stack());
            if (reference != nullptr) {
                frame.branch(offset - 3); // -3 to account for the opcode and offset bytes
            }
        } break;
        default:
            throw std::runtime_error(std::format(
                "Interpreter: unimplemented opcode 0x{:02X} at pc={} in {}.{}{}",
                opcode, frame.get_pc() - 1,
                frame.owner().this_name(), frame.method().name, frame.method().descriptor));
        }
    }

    throw std::runtime_error(std::format(
        "Interpreter: fell off the end of {}.{}{}",
        frame.owner().this_name(), frame.method().name, frame.method().descriptor));
}
