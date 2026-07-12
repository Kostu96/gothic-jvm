#include "runtime/native_methods.hpp"

#include "runtime/frame.hpp"
#include "runtime/vm.hpp"

#include <chrono>

namespace {

void not_implemented_stub(VM& vm, Frame& frame) {
    throw std::runtime_error("native method not implemented!");
}

void com_kostu96_gjvm_ResourceInputStream_init(VM& vm, Frame& frame) {
    auto* name = std::get<Object*>(frame.pop_stack());
    auto* obj = std::get<Object*>(frame.pop_stack());

    // TODO(Kostu): implement resource input stream initialization
}

void java_lang_Class_getName(VM& vm, Frame& frame) {
    auto* cls_obj = std::get<Object*>(frame.pop_stack());
    auto& mirror = std::get<ClassMirrorData>(cls_obj->data);
    Class& cls = mirror.mirrored;
    Object* str_obj = vm.heap().new_interned_string(cls.this_name());
    frame.push_stack(str_obj);
}

//void java_lang_Class_newInstance(VM& vm, Frame& frame) {
//    auto* cls_obj = std::get<Object*>(frame.pop_stack());
//    if (cls_obj == nullptr) {
//        // In a complete VM this would raise NullPointerException.
//        throw std::runtime_error("newInstance: class is null");
//    }
//    auto& mirror = std::get<ClassMirrorData>(cls_obj->data);
//    Class* cls = mirror.mirrored;
//    Object* instance = vm.heap().new_instance(*cls);
//    cls->find_method("<init>", "()V");
//    Value val = instance;
//    vm.interpreter().execute(*cls, *cls->find_method("<init>", "()V"), std::span{ &val, 1 });
//
//    frame.push_stack(instance);
//}

void java_lang_Object_getClass(VM& vm, Frame& frame) {
    auto* obj = std::get<Object*>(frame.pop_stack());

    auto& instance = std::get<InstanceData>(obj->data);
    auto* class_obj = vm.heap().class_object_for(instance.type);
    frame.push_stack(class_obj);
}

void java_lang_String_charAt(VM& vm, Frame& frame) {
    auto index = std::get<int32_t>(frame.pop_stack());
    auto* str_obj = std::get<Object*>(frame.pop_stack());
    auto& str_instance = std::get<InstanceData>(str_obj->data);
    auto* chars_obj = std::get<Object*>(str_instance.fields[0]);
    auto& char_array = std::get<PrimitiveArrayData>(chars_obj->data);
    if (index < 0 || index >= char_array.length()) {
        // In a complete VM this would raise StringIndexOutOfBoundsException.
        throw std::runtime_error("charAt: index out of bounds");
    }
    auto char_value = char_array.get(index);
    frame.push_stack(char_value);
}

void java_lang_String_indexOf(VM& vm, Frame& frame) {
    auto index = std::get<int32_t>(frame.pop_stack());
    auto char_value = std::get<int32_t>(frame.pop_stack());
    auto* str_obj = std::get<Object*>(frame.pop_stack());
    auto& str_instance = std::get<InstanceData>(str_obj->data);
    auto* chars_obj = std::get<Object*>(str_instance.fields[0]);
    auto& char_array = std::get<PrimitiveArrayData>(chars_obj->data);
    int32_t result_index = -1;
    for (int32_t i = index; i < char_array.length(); ++i) {
        if (std::get<int32_t>(char_array.get(i)) == char_value) {
            result_index = i;
            break;
        }
    }
    frame.push_stack(result_index);
}

void java_lang_String_lastIndexOf(VM& vm, Frame& frame) {
    auto char_value = std::get<int32_t>(frame.pop_stack());
    auto* str_obj = std::get<Object*>(frame.pop_stack());
    auto& str_instance = std::get<InstanceData>(str_obj->data);
    auto* chars_obj = std::get<Object*>(str_instance.fields[0]);
    auto& char_array = std::get<PrimitiveArrayData>(chars_obj->data);
    int32_t result_index = -1;
    for (int32_t i = 0; i < char_array.length(); ++i) {
        if (std::get<int32_t>(char_array.get(i)) == char_value) {
            result_index = i;
        }
    }
    frame.push_stack(result_index);
}

void java_lang_System_currentTimeMillis(VM& vm, Frame& frame) {
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    frame.push_stack(static_cast<int64_t>(ms.count()));
}

void javax_microedition_lcdui_Canvas_getHeight(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
    frame.push_stack(static_cast<int32_t>(320)); // TODO(Kostu): temp: hardcoded height
}

void javax_microedition_lcdui_Canvas_getWidth(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
    frame.push_stack(static_cast<int32_t>(240)); // TODO(Kostu): temp: hardcoded width
}

}

void javax_microedition_lcdui_Font_init(VM& vm, Frame& frame) {
    auto* font_obj = std::get<Object*>(frame.pop_stack());
    // TODO(Kostu): implement font initialization
}

void javax_microedition_lcdui_Image_init(VM& vm, Frame& frame) {
    auto height = std::get<int32_t>(frame.pop_stack());
    auto width = std::get<int32_t>(frame.pop_stack());
    auto* image_obj = std::get<Object*>(frame.pop_stack());
    // TODO(Kostu): implement image creation
}

NativeMethods::NativeMethods() {
    storage_.insert({
        { "com/kostu96/gjvm/ResourceInputStream.init(Ljava/lang/String;)V", com_kostu96_gjvm_ResourceInputStream_init },

        { "java/lang/Class.getName()Ljava/lang/String;",  java_lang_Class_getName },
        { "java/lang/Object.getClass()Ljava/lang/Class;", java_lang_Object_getClass },
        { "java/lang/String.charAt(I)C",                  java_lang_String_charAt },
        { "java/lang/String.indexOf(II)I",                java_lang_String_indexOf },
        { "java/lang/String.lastIndexOf(I)I",             java_lang_String_lastIndexOf },
        { "java/lang/System.currentTimeMillis()J",        java_lang_System_currentTimeMillis },
        
        { "javax/microedition/lcdui/Canvas.getHeight()I", javax_microedition_lcdui_Canvas_getHeight },
        { "javax/microedition/lcdui/Canvas.getWidth()I",  javax_microedition_lcdui_Canvas_getWidth },
        { "javax/microedition/lcdui/Font.init()V",        javax_microedition_lcdui_Font_init },
        { "javax/microedition/lcdui/Image.init(II)V",     javax_microedition_lcdui_Image_init }
    });
}

const NativeMethods::Callback* NativeMethods::find(std::string_view class_name,
                                                   std::string_view name,
                                                   std::string_view descriptor) const {
    std::string key = std::string(class_name) + "." + std::string(name) + std::string(descriptor);
    auto it = storage_.find(key);
    if (it != storage_.end()) {
        return &it->second;
    }

    return nullptr;
}
