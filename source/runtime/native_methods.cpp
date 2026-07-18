#include "runtime/native_methods.hpp"

#include "platform/display.hpp"
#include "runtime/frame.hpp"
#include "runtime/vm.hpp"

#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_render.h>

#include <chrono>
#include <print>

namespace {

void com_kostu96_gjvm_ResourceInputStream_init(VM& vm, Frame& frame) {
    auto& name_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    auto& name_native = std::get<StringNativeData>(name_instance.native_payload);
    auto& stream_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);

    ResourceInputStreamNativeData native_data{
        .buffer = vm.class_loader().load_resource(name_native.value),
        .position = 0
    };
    stream_instance.native_payload = native_data;
}

void com_kostu96_gjvm_ResourceInputStream_read(VM& vm, Frame& frame) {
    auto& stream_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    auto& stream_native = std::get<ResourceInputStreamNativeData>(stream_instance.native_payload);

    frame.push_stack(static_cast<int32_t>(stream_native.buffer[stream_native.position++]));
}

void java_lang_Class_getName(VM& vm, Frame& frame) {
    auto* cls_obj = std::get<Object*>(frame.pop_stack());
    auto& mirror = std::get<ClassMirrorData>(cls_obj->data);
    Class& cls = mirror.mirrored;
    Object* str_obj = vm.heap().new_interned_string(cls.this_name());
    frame.push_stack(str_obj);
}

void java_lang_Object_getClass(VM& vm, Frame& frame) {
    auto& instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);

    auto* class_obj = vm.heap().class_object_for(instance.type);
    frame.push_stack(class_obj);
}

void java_lang_String_charAt(VM& vm, Frame& frame) {
    auto index = std::get<int32_t>(frame.pop_stack());
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    auto& str_native = std::get<StringNativeData>(str_instance.native_payload);
    
    if (index < 0 || index >= str_native.value.size()) {
        // In a complete VM this would raise StringIndexOutOfBoundsException.
        throw std::runtime_error("charAt: index out of bounds");
    }
    auto ch = str_native.value[index];
    frame.push_stack(static_cast<int32_t>(ch));
}

void java_lang_String_indexOf(VM& vm, Frame& frame) {
    auto index = std::get<int32_t>(frame.pop_stack());
    auto char_value = std::get<int32_t>(frame.pop_stack());
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    auto& str_native = std::get<StringNativeData>(str_instance.native_payload);

    int32_t result_index = -1;
    for (int32_t i = index; i < str_native.value.size(); ++i) {
        if (static_cast<int32_t>(str_native.value[i]) == char_value) {
            result_index = i;
            break;
        }
    }
    frame.push_stack(result_index);
}

void java_lang_String_init_StringBuffer(VM& vm, Frame& frame) {
    auto& buffer_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    auto& buffer_native = std::get<StringNativeData>(buffer_instance.native_payload);
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    StringNativeData native_data;
    native_data.value = buffer_native.value;
    str_instance.native_payload = native_data;
}

void java_lang_String_init_char_array(VM& vm, Frame& frame) {
    auto size = std::get<int32_t>(frame.pop_stack());
    auto offset = std::get<int32_t>(frame.pop_stack());
    auto& char_array = std::get<PrimitiveArrayData>(std::get<Object*>(frame.pop_stack())->data);
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    if (offset < 0 || size < 0 || offset + size > char_array.length()) {
        // In a complete VM this would raise StringIndexOutOfBoundsException.
        throw std::runtime_error("String.init: offset and size out of bounds");
    }
    StringNativeData native_data;
    native_data.value.reserve(size);
    for (int32_t i = 0; i < size; ++i) {
        native_data.value.push_back(
            static_cast<char16_t>(std::get<int32_t>(char_array.get(offset + i))));
    }
    str_instance.native_payload = native_data;
}

void java_lang_String_lastIndexOf(VM& vm, Frame& frame) {
    auto char_value = std::get<int32_t>(frame.pop_stack());
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    auto& str_native = std::get<StringNativeData>(str_instance.native_payload);

    int32_t result_index = -1;
    for (int32_t i = 0; i < str_native.value.size(); ++i) {
        if (static_cast<int32_t>(str_native.value[i]) == char_value) {
            result_index = i;
        }
    }
    frame.push_stack(result_index);
}

void java_lang_StringBuffer_append(VM& vm, Frame& frame) {
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    auto& str_native = std::get<StringNativeData>(str_instance.native_payload);
    auto& buffer_instance = std::get<InstanceData>(std::get<Object*>(frame.peek_stack())->data);
    auto& buffer_native = std::get<StringNativeData>(buffer_instance.native_payload);

    buffer_native.value.append(str_native.value);
    auto* size_field = str_instance.type.find_field("size", "I");
    buffer_instance.fields[size_field->slot] = static_cast<int32_t>(buffer_native.value.size());
}

void java_lang_StringBuffer_init(VM& vm, Frame& frame) {
    auto& buffer_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);


    StringNativeData native_data;
    buffer_instance.native_payload = native_data;
}

void java_lang_System_currentTimeMillis(VM& vm, Frame& frame) {
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    frame.push_stack(static_cast<int64_t>(ms.count()));
}

void java_lang_Thread_start(VM& vm, Frame& frame) {
    auto& thread_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    // TODO(Kostu): implement thread start
}

void javax_microedition_lcdui_Canvas_getHeight(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
    int32_t height = vm.display()->height();
    frame.push_stack(height);
}

void javax_microedition_lcdui_Canvas_getWidth(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
    int32_t width = vm.display()->width();
    frame.push_stack(width);
}

void javax_microedition_lcdui_Font_init(VM& vm, Frame& frame) {
    auto* font_obj = std::get<Object*>(frame.pop_stack());
    // TODO(Kostu): implement font initialization
}

void javax_microedition_lcdui_Graphics_drawStringNative(VM& vm, Frame& frame) {
    auto y = std::get<int32_t>(frame.pop_stack());
    auto x = std::get<int32_t>(frame.pop_stack());
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    auto& str_native = std::get<StringNativeData>(str_instance.native_payload);
    auto& gfx_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    auto& gfx_native = std::get<GraphicsNativeData>(gfx_instance.native_payload);
    
    // TODO(Kostu): set color once
    auto* color_field = gfx_instance.type.find_field("color", "I");
    auto color = std::get<int32_t>(gfx_instance.fields[color_field->slot]);
    SDL_SetRenderDrawColor(gfx_native.sdl_renderer,
        (color >> 24) & 0xFF, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);

    SDL_RenderDebugText(gfx_native.sdl_renderer,
                        static_cast<float>(x), static_cast<float>(y),
                        str_native.value.c_str());
}

void javax_microedition_lcdui_Graphics_fillRect(VM& vm, Frame& frame) {
    auto height = std::get<int32_t>(frame.pop_stack());
    auto width = std::get<int32_t>(frame.pop_stack());
    auto y = std::get<int32_t>(frame.pop_stack());
    auto x = std::get<int32_t>(frame.pop_stack());
    auto& gfx_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    auto& gfx_native = std::get<GraphicsNativeData>(gfx_instance.native_payload);

    auto* color_field = gfx_instance.type.find_field("color", "I");
    auto color = std::get<int32_t>(gfx_instance.fields[color_field->slot]);
    SDL_SetRenderDrawColor(gfx_native.sdl_renderer,
        (color >> 24) & 0xFF, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    
    const SDL_FRect rect{
        .x = static_cast<float>(x), .y = static_cast<float>(y),
        .w = static_cast<float>(width), .h = static_cast<float>(height)
    };
    SDL_RenderFillRect(gfx_native.sdl_renderer, &rect);
}

void javax_microedition_lcdui_Graphics_init(VM& vm, Frame& frame) {
    auto& img_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    auto& img_native = std::get<ImageNativeData>(img_instance.native_payload);
    auto& gfx_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);

    GraphicsNativeData native_data{
        .sdl_renderer = SDL_CreateSoftwareRenderer(img_native.sdl_surface),
        .sdl_surface = img_native.sdl_surface
    };
    gfx_instance.native_payload = native_data;
}

void javax_microedition_lcdui_Image_getRGB(VM& vm, Frame& frame) {
    auto height = std::get<int32_t>(frame.pop_stack());
    auto width = std::get<int32_t>(frame.pop_stack());
    auto y = std::get<int32_t>(frame.pop_stack());
    auto x = std::get<int32_t>(frame.pop_stack());
    auto scan_length = std::get<int32_t>(frame.pop_stack());
    auto offset = std::get<int32_t>(frame.pop_stack());
    auto& rgb_data = std::get<PrimitiveArrayData>(std::get<Object*>(frame.pop_stack())->data);
    auto& img_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    auto& img_native = std::get<ImageNativeData>(img_instance.native_payload);

    if (SDL_MUSTLOCK(img_native.sdl_surface)) {
        SDL_LockSurface(img_native.sdl_surface);
    }
    for (int row = 0; row < height; ++row) {
        auto* src = reinterpret_cast<const Uint32*>(
            static_cast<const Uint8*>(
                img_native.sdl_surface->pixels) + (y + row) * img_native.sdl_surface->pitch);
        for (int col = 0; col < width; ++col) {
            int dst = offset + row * scan_length + col; // scanlength may be negative
            rgb_data.set(dst, static_cast<int32_t>(src[x + col]));
        }
    }
    if (SDL_MUSTLOCK(img_native.sdl_surface)) {
        SDL_UnlockSurface(img_native.sdl_surface);
    }
}

void javax_microedition_lcdui_Image_init(VM& vm, Frame& frame) {
    auto height = std::get<int32_t>(frame.pop_stack());
    auto width = std::get<int32_t>(frame.pop_stack());
    auto& img_instance = std::get<InstanceData>(std::get<Object*>(frame.pop_stack())->data);
    
    ImageNativeData native_data{
        .sdl_surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ARGB8888)
    };
    img_instance.native_payload = native_data;
}

}

NativeMethods::NativeMethods() {
    storage_.insert({
        { "com/kostu96/gjvm/ResourceInputStream.init(Ljava/lang/String;)V",
           com_kostu96_gjvm_ResourceInputStream_init },
        { "com/kostu96/gjvm/ResourceInputStream.read()I",
           com_kostu96_gjvm_ResourceInputStream_read },

        { "java/lang/Class.getName()Ljava/lang/String;", java_lang_Class_getName },
        { "java/lang/Object.getClass()Ljava/lang/Class;", java_lang_Object_getClass },
        { "java/lang/String.charAt(I)C", java_lang_String_charAt },
        { "java/lang/String.indexOf(II)I", java_lang_String_indexOf },
        { "java/lang/String.init(Ljava/lang/StringBuffer;)V", java_lang_String_init_StringBuffer },
        { "java/lang/String.init([CII)V", java_lang_String_init_char_array },
        { "java/lang/String.lastIndexOf(I)I", java_lang_String_lastIndexOf },
        { "java/lang/StringBuffer.append(Ljava/lang/String;)Ljava/lang/StringBuffer;", java_lang_StringBuffer_append },
        { "java/lang/StringBuffer.init()V", java_lang_StringBuffer_init },
        { "java/lang/System.currentTimeMillis()J", java_lang_System_currentTimeMillis },
        { "java/lang/Thread.start()V", java_lang_Thread_start },
        
        { "javax/microedition/lcdui/Canvas.getHeight()I", javax_microedition_lcdui_Canvas_getHeight },
        { "javax/microedition/lcdui/Canvas.getWidth()I", javax_microedition_lcdui_Canvas_getWidth },
        { "javax/microedition/lcdui/Font.init()V", javax_microedition_lcdui_Font_init },
        { "javax/microedition/lcdui/Graphics.drawStringNative(Ljava/lang/String;II)V",
           javax_microedition_lcdui_Graphics_drawStringNative },
        { "javax/microedition/lcdui/Graphics.fillRect(IIII)V", javax_microedition_lcdui_Graphics_fillRect },
        { "javax/microedition/lcdui/Graphics.init(Ljavax/microedition/lcdui/Image;)V",
           javax_microedition_lcdui_Graphics_init },
        { "javax/microedition/lcdui/Image.getRGB([IIIIIII)V", javax_microedition_lcdui_Image_getRGB },
        { "javax/microedition/lcdui/Image.init(II)V", javax_microedition_lcdui_Image_init }
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
