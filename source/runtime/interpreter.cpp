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

// TODO(Kostu): move this to utils
//template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
//template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void Interpreter::run(Thread& thread, size_t num_instructions) {
    while (num_instructions-- > 0 && !thread.is_terminated()) {
        if (vm_.stop_requested()) {
            throw VmStopRequested{};
        }
        auto& frame = thread.current_frame();
        frame.record_last_pc();

        const auto opcode = frame.pop_code_u8();
        switch (opcode) {
        case op_nop: break;
        case op_aconst_null: {
            frame.push_stack(static_cast<Object*>(nullptr));
        } break;
        case op_iconst_m1: {
            frame.push_stack(static_cast<int32_t>(-1));
        } break;
        case op_iconst_0: {
            frame.push_stack(static_cast<int32_t>(0));
        } break;
        case op_iconst_1: {
            frame.push_stack(static_cast<int32_t>(1));
        } break;
        case op_iconst_2: {
            frame.push_stack(static_cast<int32_t>(2));
        } break;
        case op_iconst_3: {
            frame.push_stack(static_cast<int32_t>(3));
        } break;
        case op_iconst_4: {
            frame.push_stack(static_cast<int32_t>(4));
        } break;
        case op_iconst_5: {
            frame.push_stack(static_cast<int32_t>(5));
        } break;
        case op_lconst_0: {
            frame.push_stack(static_cast<int64_t>(0));
        } break;
        case op_lconst_1: {
            frame.push_stack(static_cast<int64_t>(1));
        } break;
        case op_fconst_0: {
            frame.push_stack(static_cast<float>(0.0));
        } break;
        case op_fconst_1: {
            frame.push_stack(static_cast<float>(1.0));
        } break;
        case op_fconst_2: {
            frame.push_stack(static_cast<float>(2.0));
        } break;
        case op_dconst_0: {
            frame.push_stack(static_cast<double>(0.0));
        } break;
        case op_dconst_1: {
            frame.push_stack(static_cast<double>(1.0));
        } break;
        case op_bipush: {
            auto value = frame.pop_code_u8();
            frame.push_stack(static_cast<int32_t>(value));
        } break;
        case op_sipush: {
            auto value = frame.pop_code_u16();
            frame.push_stack(static_cast<int32_t>(value));
        } break;
        case op_ldc: {
            const auto index = frame.pop_code_u8();
            const auto value = frame.owner().resolve_constant(index, vm_.class_loader(), vm_.heap());
            frame.push_stack(value);
        } break;
        case op_ldc_w: {
            const auto index = frame.pop_code_u16();
            const auto value = frame.owner().resolve_constant(index, vm_.class_loader(), vm_.heap());
            frame.push_stack(value);
        } break;
        case op_ldc2_w: {
            const auto index = frame.pop_code_u16();
            const auto value = frame.owner().resolve_constant(index, vm_.class_loader(), vm_.heap());
            frame.push_stack(value);
        } break;
        case op_iload: {
            auto index = frame.pop_code_u8();
            frame.push_stack(frame.locals()[index]);
        } break;

        case op_aload: {
            auto index = frame.pop_code_u8();
            frame.push_stack(frame.locals()[index]);
        } break;
        case op_iload_0: {
            frame.push_stack(frame.locals()[0]);
        } break;
        case op_iload_1: {
            frame.push_stack(frame.locals()[1]);
        } break;
        case op_iload_2: {
            frame.push_stack(frame.locals()[2]);
        } break;
        case op_iload_3: {
            frame.push_stack(frame.locals()[3]);
        } break;
        case op_lload_0: {
            frame.push_stack(frame.locals()[0]);
        } break;
        case op_lload_1: {
            frame.push_stack(frame.locals()[1]);
        } break;
        case op_lload_2: {
            frame.push_stack(frame.locals()[2]);
        } break;
        case op_lload_3: {
            frame.push_stack(frame.locals()[3]);
        } break;

        case op_aload_0: {
            frame.push_stack(frame.locals()[0]);
        } break;
        case op_aload_1: {
            frame.push_stack(frame.locals()[1]);
        } break;
        case op_aload_2: {
            frame.push_stack(frame.locals()[2]);
        } break;
        case op_aload_3: {
            frame.push_stack(frame.locals()[3]);
        } break;
        case op_iaload: {
            auto index = std::get<int32_t>(frame.pop_stack());
            auto* reference = std::get<Object*>(frame.pop_stack());
            auto* array = reference ? std::get_if<PrimitiveArrayData>(&reference->data) : nullptr;
            if (array == nullptr) {
                // In a complete VM a null reference would raise NullPointerException.
                throw std::runtime_error("caload: object is not an array reference");
            }
            frame.push_stack(array->get(index));
        } break;

        case op_aaload: {
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
            frame.locals()[index] = frame.pop_stack();
        } break;

        case op_astore: {
            auto index = frame.pop_code_u8();
            frame.locals()[index] = frame.pop_stack();
        } break;
        case op_istore_0: {
            frame.locals()[0] = frame.pop_stack();
        } break;
        case op_istore_1: {
            frame.locals()[1] = frame.pop_stack();
        } break;
        case op_istore_2: {
            frame.locals()[2] = frame.pop_stack();
        } break;
        case op_istore_3: {
            frame.locals()[3] = frame.pop_stack();
        } break;

        case op_astore_0: {
            auto value = std::get<Object*>(frame.pop_stack());
            frame.locals()[0] = value;
        } break;
        case op_astore_1: {
            auto value = std::get<Object*>(frame.pop_stack());
            frame.locals()[1] = value;
        } break;
        case op_astore_2: {
            auto value = std::get<Object*>(frame.pop_stack());
            frame.locals()[2] = value;
        } break;
        case op_astore_3: {
            auto value = std::get<Object*>(frame.pop_stack());
            frame.locals()[3] = value;
        } break;
        case op_iastore: {
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
        case op_pop: {
            frame.pop_stack();
        } break;
        
        case op_dup: {
            frame.push_stack(frame.peek_stack());
        } break;

        case op_dup2: {
            auto value1 = frame.peek_stack();
            if (std::holds_alternative<int64_t>(value1) || std::holds_alternative<double>(value1)) {
                // If the top value is a long or double, we only need to duplicate it once.
                frame.push_stack(value1);
            }
            else {
                auto value2 = frame.peek_stack(1);
                frame.push_stack(value2);
                frame.push_stack(value1);
            }
        } break;

        case op_iadd: {
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(value1 + value2);
        } break;

        case op_isub: {
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(value1 - value2);
        } break;

        case op_imul: {
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(value1 * value2);
        } break;

        case op_idiv: {
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            if (value2 == 0) {
                // In a complete VM this would raise ArithmeticException.
                throw std::runtime_error("idiv: division by zero");
            }
            frame.push_stack(value1 / value2);
        } break;

        case op_irem: {
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            if (value2 == 0) {
                // In a complete VM this would raise ArithmeticException.
                throw std::runtime_error("irem: division by zero");
            }
            frame.push_stack(value1 % value2);
        } break;

        case op_ineg: {
            auto value = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(-value);
        } break;

        case op_ishl: {
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(value1 << (value2 & 0x1F));
        } break;

        case op_ishr: {
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(value1 >> (value2 & 0x1F));
        } break;

        case op_iand: {
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(value1 & value2);
        } break;
        case op_land: {
            auto value2 = std::get<int64_t>(frame.pop_stack());
            auto value1 = std::get<int64_t>(frame.pop_stack());
            frame.push_stack(value1 & value2);
        } break;
        case op_ior: {
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(value1 | value2);
        } break;

        case op_lxor: {
            auto value2 = std::get<int64_t>(frame.pop_stack());
            auto value1 = std::get<int64_t>(frame.pop_stack());
            frame.push_stack(value1 ^ value2);
        } break;
        case op_iinc: {
            auto index = frame.pop_code_u8();
            auto constant = static_cast<int8_t>(frame.pop_code_u8());
            frame.locals()[index] = std::get<int32_t>(frame.locals()[index]) + constant;
        } break;

        case op_i2b: {
            auto value = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(static_cast<int32_t>(static_cast<int8_t>(value)));
        } break;

        case op_i2s: {
            auto value = std::get<int32_t>(frame.pop_stack());
            frame.push_stack(static_cast<int32_t>(static_cast<int16_t>(value)));
        } break;

        case op_ifeq: {
            auto offset = static_cast<int16_t>(frame.pop_code_u16());
            auto value = std::get<int32_t>(frame.pop_stack());
            if (value == 0) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_ifne: {
            auto offset = static_cast<int16_t>(frame.pop_code_u16());
            auto value = std::get<int32_t>(frame.pop_stack());
            if (value != 0) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_iflt: {
            auto offset = static_cast<int16_t>(frame.pop_code_u16());
            auto value = std::get<int32_t>(frame.pop_stack());
            if (value < 0) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_ifge: {
            auto offset = static_cast<int16_t>(frame.pop_code_u16());
            auto value = std::get<int32_t>(frame.pop_stack());
            if (value >= 0) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;

        case op_ifle: {
            auto offset = static_cast<int16_t>(frame.pop_code_u16());
            auto value = std::get<int32_t>(frame.pop_stack());
            if (value <= 0) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_if_icmpeq: {
            auto offset = static_cast<int16_t>(frame.pop_code_u16());
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            if (value1 == value2) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_if_icmpne: {
            auto offset = static_cast<int16_t>(frame.pop_code_u16());
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            if (value1 != value2) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_if_icmplt: {
            auto offset = static_cast<int16_t>(frame.pop_code_u16());
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            if (value1 < value2) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_if_icmpge: {
            auto offset = static_cast<int16_t>(frame.pop_code_u16());
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            if (value1 >= value2) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_if_icmpgt: {
            auto offset = static_cast<int16_t>(frame.pop_code_u16());
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            if (value1 > value2) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;
        case op_if_icmple: {
            auto offset = static_cast<int16_t>(frame.pop_code_u16());
            auto value2 = std::get<int32_t>(frame.pop_stack());
            auto value1 = std::get<int32_t>(frame.pop_stack());
            if (value1 <= value2) {
                frame.branch(offset - 3); // -3 to account for the size of the instruction itself
            }
        } break;

        case op_goto: {
            auto offset = static_cast<int16_t>(frame.pop_code_u16());
            frame.branch(offset - 3); // -3 to account for the size of the instruction itself
        } break;

        case op_ireturn: {
            if (frame.operand_stack().size() != 1) {
                throw std::runtime_error("ireturn: operand stack should have exactly one value");
            }
            Value ret = frame.pop_stack();
            thread.pop_frame();
            thread.current_frame().push_stack(ret);
        } break;

        case op_areturn: {
            if (frame.operand_stack().size() != 1) {
                throw std::runtime_error("areturn: operand stack should have exactly one value");
            }
            Value ret = frame.pop_stack();
            thread.pop_frame();
            thread.current_frame().push_stack(ret);
        } break;
        case op_return: {
            if (!frame.operand_stack().empty()) {
                throw std::runtime_error("return: operand stack should be empty");
            }
            if (frame.method().is_class_initializer) {
                frame.method().owner.set_initialized();
            }
            thread.pop_frame();
        } break;
        case op_getstatic: {
            auto index = frame.pop_code_u16();
            auto& field = frame.owner().resolve_field(index, vm_.class_loader());
            if (!field.is_static) {
                throw std::runtime_error(std::format(
                    "getstatic: field {}.{}:{} is not static",
                    field.owner.this_name(), field.name, field.descriptor));
            }

            if (field.owner.needs_initialization()) {
                frame.rewind_pc();
                field.owner.ensure_initialized(thread);
            }
            else {
                frame.push_stack(field.value);
            }
        } break;
        case op_putstatic: {
            auto index = frame.pop_code_u16();
            auto& field = frame.owner().resolve_field(index, vm_.class_loader());
            if (!field.is_static) {
                throw std::runtime_error(std::format(
                    "putstatic: field {}.{}:{} is not static",
                    field.owner.this_name(), field.name, field.descriptor));
            }

            if (field.owner.needs_initialization()) {
                frame.rewind_pc();
                field.owner.ensure_initialized(thread);
            }
            else {
                field.value = frame.pop_stack();
            }
        } break;
        case op_getfield: {
            auto index = frame.pop_code_u16();
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
            auto index = frame.pop_code_u16();
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
            auto index = frame.pop_code_u16();
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

            invoke(thread, *method);
        } break;
        case op_invokespecial: {
            auto index = frame.pop_code_u16();
            const Method& method = frame.owner().resolve_method(index, vm_.class_loader());
            bool is_constructor = (method.name == "<init>");
            bool is_interface = method.owner.is_interface();
            bool is_superclass = (&method.owner == frame.owner().super());
            if (!is_constructor && !is_interface && is_superclass && frame.owner().treat_super_specially()) {
                throw std::runtime_error("invokespecial: ACC_SUPER semantics not implemented yet");
            }

            invoke(thread, method);
        } break;
        case op_invokestatic: {
            auto index = frame.pop_code_u16();
            const Method& method = frame.owner().resolve_method(index, vm_.class_loader());
            if (!method.is_static) {
                throw std::runtime_error(std::format(
                    "invokestatic: method {}.{}{} is not static",
                    method.owner.this_name(), method.name, method.descriptor));
            }

            if (method.owner.needs_initialization()) {
                frame.rewind_pc();
                method.owner.ensure_initialized(thread);
            }
            else {
                invoke(thread, method);
            }
        } break;

        case op_new: {
            auto index = frame.pop_code_u16();
            Class& target = frame.owner().resolve_class(index, vm_.class_loader());
            if (target.needs_initialization()) {
                frame.rewind_pc();
                target.ensure_initialized(thread);
            }
            else {
                Object* instance = vm_.heap().new_instance(target);
                frame.push_stack(instance);
            }
        } break;
        case op_newarray: {
            auto type = frame.pop_code_u8();
            auto count = std::get<int32_t>(frame.pop_stack());
            Object* array =
                vm_.heap().new_primitive_array(static_cast<PrimitiveArrayData::ElementType>(type), count);
            frame.push_stack(array);
        } break;
        case op_anewarray: {
            auto index = frame.pop_code_u16();
            Class& target = frame.owner().resolve_class(index, vm_.class_loader());

            auto count = std::get<int32_t>(frame.pop_stack());
            Object* array = vm_.heap().new_instance_array(target, count);
            frame.push_stack(array);
        } break;
        case op_arraylength: {
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
        case op_athrow: {
            auto* exc = std::get<Object*>(frame.pop_stack());
            if (exc == nullptr) {
                // In a complete VM this would raise NullPointerException.
                throw std::runtime_error("athrow: exception object is null");
            }
            thread.set_pending_exception(exc);
        } break;
        case op_checkcast: {
            auto index = frame.pop_code_u16();
            Class& target = frame.owner().resolve_class(index, vm_.class_loader());
            // NOOP for now but resolves a class
        } break;

        case op_monitorenter: {
            auto* reference = std::get<Object*>(frame.pop_stack());
            if (reference == nullptr) {
                // In a complete VM this would raise NullPointerException.
                throw std::runtime_error("monitorenter: object is null");
            }
            
            if (reference->monitor.owner == nullptr) {
                reference->monitor.owner = &thread;
                reference->monitor.recursion_count = 1;
            }
            else if (reference->monitor.owner == &thread) {
                reference->monitor.recursion_count++;
            }
            else {
                // In a complete VM this would block the thread until the monitor is available.
                throw std::runtime_error("monitorenter: monitor is already owned by another thread");
            }
        } break;
        case op_monitorexit: {
            auto* reference = std::get<Object*>(frame.pop_stack());
            if (reference == nullptr) {
                // In a complete VM this would raise NullPointerException.
                throw std::runtime_error("monitorexit: object is null");
            }

            if (reference->monitor.owner == &thread) {
                reference->monitor.recursion_count--;
                if (reference->monitor.recursion_count == 0) {
                    reference->monitor.owner = nullptr;
                }
            }
            else {
                // In a complete VM this would raise IllegalMonitorStateException.
                throw std::runtime_error("monitorexit: monitor is not owned by the current thread");
            }
        } break;

        case op_multianewarray: {
            auto index = frame.pop_code_u16();
            auto dimensions = frame.pop_code_u8();
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
            auto offset = frame.pop_code_u16();
            auto* reference = std::get<Object*>(frame.pop_stack());
            if (reference == nullptr) {
                frame.branch(offset - 3); // -3 to account for the opcode and offset bytes
            }
        } break;
        case op_ifnonnull: {
            auto offset = frame.pop_code_u16();
            auto* reference = std::get<Object*>(frame.pop_stack());
            if (reference != nullptr) {
                frame.branch(offset - 3); // -3 to account for the opcode and offset bytes
            }
        } break;
        default:
            throw std::runtime_error(std::format(
                "Interpreter: unimplemented opcode 0x{:02X} at pc={} in {}.{}{}",
                opcode, frame.last_pc(),
                frame.owner().this_name(), frame.method().name, frame.method().descriptor));
        }

        if (thread.has_pending_exception()) {
            dispatch_pending_exception(thread);
        }
    }
}

void Interpreter::invoke(Thread& thread, const Method& method) {
    if (method.is_native) {
        std::println(
            "Interpreter: executing native {}.{}{}", method.owner.this_name(), method.name, method.descriptor);
        if (method.native_callback) {
            (*(method.native_callback))(vm_, thread);
        }
        else {
            throw std::runtime_error(std::format(
                "Failed to call native: {}.{}{}", method.owner.this_name(), method.name, method.descriptor));
        }
    }
    else {
        std::vector<Value> args(method.arg_slot_widths.size());
        for (size_t i = method.arg_slot_widths.size(); i > 0; --i) {
            args[i - 1] = thread.current_frame().pop_stack();
        }
        thread.push_frame(method, args);
    }
}

void Interpreter::dispatch_pending_exception(Thread& thread) {
    Object* exc = thread.pending_exception();
    Class& exc_class = std::get<InstanceData>(exc->data).type;

    while (!thread.is_terminated()) {
        Frame& frame = thread.current_frame();
        size_t pc = frame.last_pc();
        for (const auto& e : frame.method().exception_table) {
            if (pc < e.start_pc || pc >= e.end_pc) continue;
            bool matches = (e.catch_type == 0); // 0 == catch-all (finally)
            if (!matches) {
                Class& handler_type =
                    frame.owner().resolve_class(e.catch_type, vm_.class_loader());
                matches = exc_class.is_subclass_of(handler_type);
            }
            if (matches) {
                frame.operand_stack().clear();
                frame.push_stack(exc);
                frame.set_pc(e.handler_pc);
                thread.clear_pending_exception();
                return;
            }
        }
        thread.pop_frame();
    }
    
    throw std::runtime_error(std::format("Uncaught exception: {}", exc_class.this_name()));
}
