#include "runtime/native_methods.hpp"

#include "platform/display.hpp"
#include "runtime/frame.hpp"
#include "runtime/vm.hpp"

#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_render.h>

#include <stb/stb_image.h>

#include <chrono>
#include <print>

namespace {

void com_kostu96_gjvm_io_ResourceInputStream_init(VM& vm, Thread& thread) {
    auto& name_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& name_native = std::get<StringNativeData>(name_instance.native_payload);
    auto& stream_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);

    try {
        ResourceInputStreamNativeData native_data{
            .buffer = vm.class_loader().load_resource(name_native.value),
            .position = 0
        };
        stream_instance.native_payload = native_data;
    }
    catch (const std::exception& e) {
        std::println("ResourceInputStream.init: failed to load resource '{}': {}", name_native.value, e.what());
        thread.set_pending_exception(vm.heap().new_instance(vm.class_loader().load("java/io/IOException")));
    }
}

void com_kostu96_gjvm_io_ResourceInputStream_read(VM& vm, Thread& thread) {
    auto& stream_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& stream_native = std::get<ResourceInputStreamNativeData>(stream_instance.native_payload);

    if (stream_native.position >= stream_native.buffer.size()) {
        thread.current_frame().push_stack(-1);
        return;
    }

    thread.current_frame().push_stack(static_cast<int32_t>(stream_native.buffer[stream_native.position++]));
}

void java_lang_Class_forName(VM& vm, Thread& thread) {
    auto& name_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& name_native = std::get<StringNativeData>(name_instance.native_payload);
    std::string name_copy = name_native.value;
    std::replace(name_copy.begin(), name_copy.end(), '.', '/');
    Class& cls = vm.class_loader().load(name_copy);
    Object* cls_obj = vm.heap().class_object_for(cls);
    thread.current_frame().push_stack(cls_obj);
}

void java_lang_Class_getName(VM& vm, Thread& thread) {
    auto* cls_obj = std::get<Object*>(thread.current_frame().pop_stack());
    auto& mirror = std::get<ClassMirrorData>(cls_obj->data);
    Class& cls = mirror.mirrored;
    Object* str_obj = vm.heap().new_interned_string(cls.this_name());
    thread.current_frame().push_stack(str_obj);
}

void java_lang_Object_getClass(VM& vm, Thread& thread) {
    auto& instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);

    auto* class_obj = vm.heap().class_object_for(instance.type);
    thread.current_frame().push_stack(class_obj);
}

void java_lang_String_charAt(VM& vm, Thread& thread) {
    auto index = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& str_native = std::get<StringNativeData>(str_instance.native_payload);
    
    if (index < 0 || index >= str_native.value.size()) {
        // In a complete VM this would raise StringIndexOutOfBoundsException.
        throw std::runtime_error("charAt: index out of bounds");
    }
    auto ch = str_native.value[index];
    thread.current_frame().push_stack(static_cast<int32_t>(ch));
}

void java_lang_String_indexOf(VM& vm, Thread& thread) {
    auto index = std::get<int32_t>(thread.current_frame().pop_stack());
    auto char_value = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& str_native = std::get<StringNativeData>(str_instance.native_payload);

    int32_t result_index = -1;
    for (int32_t i = index; i < str_native.value.size(); ++i) {
        if (static_cast<int32_t>(str_native.value[i]) == char_value) {
            result_index = i;
            break;
        }
    }
    thread.current_frame().push_stack(result_index);
}

void java_lang_String_init_byte_array(VM& vm, Thread& thread) {
    auto size = std::get<int32_t>(thread.current_frame().pop_stack());
    auto offset = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& byte_array = std::get<PrimitiveArrayData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    if (offset < 0 || size < 0 || offset + size > byte_array.length()) {
        // In a complete VM this would raise StringIndexOutOfBoundsException.
        throw std::runtime_error("String.init: offset and size out of bounds");
    }
    StringNativeData native_data;
    native_data.value.reserve(size);
    for (int32_t i = 0; i < size; ++i) {
        native_data.value.push_back(
            static_cast<char16_t>(std::get<int32_t>(byte_array.get(offset + i))));
    }
    str_instance.native_payload = native_data;
}

void java_lang_String_init_StringBuffer(VM& vm, Thread& thread) {
    auto& buffer_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& buffer_native = std::get<StringNativeData>(buffer_instance.native_payload);
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    StringNativeData native_data;
    native_data.value = buffer_native.value;
    str_instance.native_payload = native_data;
}

void java_lang_String_init_char_array(VM& vm, Thread& thread) {
    auto size = std::get<int32_t>(thread.current_frame().pop_stack());
    auto offset = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& char_array = std::get<PrimitiveArrayData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
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

void java_lang_String_lastIndexOf(VM& vm, Thread& thread) {
    auto char_value = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& str_native = std::get<StringNativeData>(str_instance.native_payload);

    int32_t result_index = -1;
    for (int32_t i = 0; i < str_native.value.size(); ++i) {
        if (static_cast<int32_t>(str_native.value[i]) == char_value) {
            result_index = i;
        }
    }
    thread.current_frame().push_stack(result_index);
}

void java_lang_String_replace(VM& vm, Thread& thread) {
    auto new_char = std::get<int32_t>(thread.current_frame().pop_stack());
    auto old_char = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& str_native = std::get<StringNativeData>(str_instance.native_payload);
    std::string new_str;
    new_str.reserve(str_native.value.size());
    for (char16_t ch : str_native.value) {
        if (static_cast<int32_t>(ch) == old_char) {
            new_str.push_back(static_cast<char16_t>(new_char));
        }
        else {
            new_str.push_back(ch);
        }
    }
    Object* new_str_obj = vm.heap().new_interned_string(new_str);
    thread.current_frame().push_stack(new_str_obj);
}

void java_lang_String_substringNative(VM& vm, Thread& thread) {
    auto end_index = std::get<int32_t>(thread.current_frame().pop_stack());
    auto begin_index = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& str_native = std::get<StringNativeData>(str_instance.native_payload);
    if (begin_index < 0 || end_index > str_native.value.size() || begin_index > end_index) {
        // In a complete VM this would raise StringIndexOutOfBoundsException.
        throw std::runtime_error("substring: index out of bounds");
    }
    auto substr = std::string_view(str_native.value).substr(begin_index, end_index - begin_index);
    Object* substr_obj = vm.heap().new_interned_string(substr);
    thread.current_frame().push_stack(substr_obj);
}

void java_lang_String_toUpperCase(VM& vm, Thread& thread) {
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& str_native = std::get<StringNativeData>(str_instance.native_payload);
    std::string upper_str;
    upper_str.reserve(str_native.value.size());
    for (char16_t ch : str_native.value) {
        upper_str.push_back(static_cast<char16_t>(std::toupper(ch)));
    }
    Object* upper_str_obj = vm.heap().new_interned_string(upper_str);
    thread.current_frame().push_stack(upper_str_obj);
}

void java_lang_StringBuffer_append(VM& vm, Thread& thread) {
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& str_native = std::get<StringNativeData>(str_instance.native_payload);
    auto& buffer_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().peek_stack())->data);
    auto& buffer_native = std::get<StringNativeData>(buffer_instance.native_payload);

    buffer_native.value.append(str_native.value);
    auto* size_field = str_instance.type.find_field("size", "I");
    buffer_instance.fields[size_field->slot] = static_cast<int32_t>(buffer_native.value.size());
}

void java_lang_StringBuffer_init(VM& vm, Thread& thread) {
    auto& buffer_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);


    StringNativeData native_data;
    buffer_instance.native_payload = native_data;
}

void java_lang_System_arraycopy(VM& vm, Thread& thread) {
    auto length = std::get<int32_t>(thread.current_frame().pop_stack());
    auto dst_offset = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& dst_array = std::get<PrimitiveArrayData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto src_offset = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& src_array = std::get<PrimitiveArrayData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    if (src_offset < 0 || dst_offset < 0 || length < 0 ||
        src_offset + length > src_array.length() ||
        dst_offset + length > dst_array.length()) {
        // In a complete VM this would raise ArrayIndexOutOfBoundsException.
        throw std::runtime_error("System.arraycopy: index out of bounds");
    }
    for (int32_t i = 0; i < length; ++i) {
        Value value = src_array.get(src_offset + i);
        dst_array.set(dst_offset + i, value);
    }
}

void java_lang_System_currentTimeMillis(VM& vm, Thread& thread) {
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    thread.current_frame().push_stack(static_cast<int64_t>(ms.count()));
}

void java_lang_System_getProperty(VM& vm, Thread& thread) {
    auto& key_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& key_native = std::get<StringNativeData>(key_instance.native_payload);
    std::string value;
    if (key_native.value == "microedition.locale") {
        //value = "en-US";
        //Object* value_obj = vm.heap().new_interned_string(value);
        //thread.current_frame().push_stack(value_obj);
        thread.current_frame().push_stack(nullptr);
    }
    else {
        throw std::runtime_error(std::format("System.getProperty: unknown property key '{}'", key_native.value));
    }

}

void java_lang_Thread_start(VM& vm, Thread& thread) {
    auto thread_obj = thread.current_frame().pop_stack();
    auto& thread_instance = std::get<InstanceData>(std::get<Object*>(thread_obj)->data);
    
    auto& new_thread = vm.create_thread();
    auto run_method = thread_instance.type.find_method("run", "()V");
    new_thread.push_frame(*run_method, std::span{ &thread_obj, 1 });
}

void javax_microedition_lcdui_Canvas_getHeight(VM& vm, Thread& thread) {
    thread.current_frame().pop_stack(); // this
    int32_t height = vm.display()->height();
    thread.current_frame().push_stack(height);
}

void javax_microedition_lcdui_Canvas_getWidth(VM& vm, Thread& thread) {
    thread.current_frame().pop_stack(); // this
    int32_t width = vm.display()->width();
    thread.current_frame().push_stack(width);
}

void javax_microedition_lcdui_Canvas_repaint(VM& vm, Thread& thread) {
    auto height = std::get<int32_t>(thread.current_frame().pop_stack());
    auto width = std::get<int32_t>(thread.current_frame().pop_stack());
    auto y = std::get<int32_t>(thread.current_frame().pop_stack());
    auto x = std::get<int32_t>(thread.current_frame().pop_stack());
    thread.current_frame().pop_stack(); // this
    //vm.display()->repaint(x, y, width, height);
}

void javax_microedition_lcdui_Canvas_serviceRepaints(VM& vm, Thread& thread) {
    thread.current_frame().pop_stack(); // this
    //vm.display()->service_repaints();
}

void javax_microedition_lcdui_Font_init(VM& vm, Thread& thread) {
    auto* font_obj = std::get<Object*>(thread.current_frame().pop_stack());
    // TODO(Kostu): implement font initialization
}

void javax_microedition_lcdui_Graphics_drawStringNative(VM& vm, Thread& thread) {
    auto y = std::get<int32_t>(thread.current_frame().pop_stack());
    auto x = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& str_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& str_native = std::get<StringNativeData>(str_instance.native_payload);
    auto& gfx_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
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

void javax_microedition_lcdui_Graphics_fillRect(VM& vm, Thread& thread) {
    auto height = std::get<int32_t>(thread.current_frame().pop_stack());
    auto width = std::get<int32_t>(thread.current_frame().pop_stack());
    auto y = std::get<int32_t>(thread.current_frame().pop_stack());
    auto x = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& gfx_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
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

void javax_microedition_lcdui_Graphics_init(VM& vm, Thread& thread) {
    auto& img_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& img_native = std::get<ImageNativeData>(img_instance.native_payload);
    auto& gfx_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);

    GraphicsNativeData native_data{
        .sdl_renderer = SDL_CreateSoftwareRenderer(img_native.sdl_surface),
        .sdl_surface = img_native.sdl_surface
    };
    gfx_instance.native_payload = native_data;
}

void javax_microedition_lcdui_Image_createImage_byte_array(VM& vm, Thread& thread) {
    auto size = std::get<int32_t>(thread.current_frame().pop_stack());
    auto offset = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& byte_array = std::get<PrimitiveArrayData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    if (offset < 0 || size < 0 || offset + size > byte_array.length()) {
        // In a complete VM this would raise ArrayIndexOutOfBoundsException.
        throw std::runtime_error("Image.createImage: offset and size out of bounds");
    }
    auto& elements = std::get<std::vector<uint8_t>>(byte_array.elements);

    int width, height, channels;
    auto pixels = stbi_load_from_memory(elements.data() + offset, size, &width, &height, &channels, 4);
    for (int i = 0; i < width * height; ++i) {
        uint8_t* pixel = reinterpret_cast<uint8_t*>(pixels) + i * 4;
        // Swap RGBA to ARGB:
        std::swap(pixel[0], pixel[3]); // R <-> A AGBR
        std::swap(pixel[1], pixel[3]); // G <-> R ARBG
        std::swap(pixel[2], pixel[3]); // B <-> G ARGB
    }
    ImageNativeData native_data{
        .sdl_surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_ARGB8888, pixels, width * 4)
    };
    auto* img_obj = vm.heap().new_instance(vm.class_loader().load("javax/microedition/lcdui/Image"));
    auto& img_instance = std::get<InstanceData>(img_obj->data);
    img_instance.native_payload = native_data;
    thread.current_frame().push_stack(img_obj);
}

void javax_microedition_lcdui_Image_createImage_InputStream(VM& vm, Thread& thread) {
    auto& stream_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& stream_native = std::get<ResourceInputStreamNativeData>(stream_instance.native_payload);

    int width, height, channels;
    auto pixels = stbi_load_from_memory(
        stream_native.buffer.data(), static_cast<int>(stream_native.buffer.size()), &width, &height, &channels, 4);

    for (int i = 0; i < width * height; ++i) {
        uint8_t* pixel = reinterpret_cast<uint8_t*>(pixels) + i * 4;
        // Swap RGBA to ARGB:
        std::swap(pixel[0], pixel[3]); // R <-> A AGBR
        std::swap(pixel[1], pixel[3]); // G <-> R ARBG
        std::swap(pixel[2], pixel[3]); // B <-> G ARGB
    }

    ImageNativeData native_data{
        .sdl_surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_ARGB8888, pixels, width * 4)
    };

    auto* img_obj = vm.heap().new_instance(vm.class_loader().load("javax/microedition/lcdui/Image"));
    auto& img_instance = std::get<InstanceData>(img_obj->data);
    img_instance.native_payload = native_data;

    thread.current_frame().push_stack(img_obj);
}

void javax_microedition_lcdui_Image_getHeight(VM& vm, Thread& thread) {
    auto& img_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& img_native = std::get<ImageNativeData>(img_instance.native_payload);
    int32_t height = static_cast<int32_t>(img_native.sdl_surface->h);
    thread.current_frame().push_stack(height);
}

void javax_microedition_lcdui_Image_getRGB(VM& vm, Thread& thread) {
    auto height = std::get<int32_t>(thread.current_frame().pop_stack());
    auto width = std::get<int32_t>(thread.current_frame().pop_stack());
    auto y = std::get<int32_t>(thread.current_frame().pop_stack());
    auto x = std::get<int32_t>(thread.current_frame().pop_stack());
    auto scan_length = std::get<int32_t>(thread.current_frame().pop_stack());
    auto offset = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& rgb_data = std::get<PrimitiveArrayData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& img_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
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

void javax_microedition_lcdui_Image_getWidth(VM& vm, Thread& thread) {
    auto& img_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& img_native = std::get<ImageNativeData>(img_instance.native_payload);
    int32_t width = static_cast<int32_t>(img_native.sdl_surface->w);
    thread.current_frame().push_stack(width);
}

void javax_microedition_lcdui_Image_init(VM& vm, Thread& thread) {
    auto height = std::get<int32_t>(thread.current_frame().pop_stack());
    auto width = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& img_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    
    ImageNativeData native_data{
        .sdl_surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ARGB8888)
    };
    img_instance.native_payload = native_data;
}

void javax_microedition_rms_RecordStore_addRecord(VM& vm, Thread& thread) {
    auto size = std::get<int32_t>(thread.current_frame().pop_stack());
    auto offset = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& data_array = std::get<PrimitiveArrayData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& record_store_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& record_store_native = std::get<RecordStoreNativeData>(record_store_instance.native_payload);
    
    if (offset < 0 || size < 0 || offset + size > data_array.length()) {
        // In a complete VM this would raise ArrayIndexOutOfBoundsException.
        throw std::runtime_error("RecordStore.addRecord: offset and size out of bounds");
    }

    auto& elements = std::get<std::vector<uint8_t>>(data_array.elements);
    int32_t record_id = record_store_native.record_store->add_record(
        std::span<const uint8_t>(elements.data() + offset, size));
    thread.current_frame().push_stack(record_id);
}

void javax_microedition_rms_RecordStore_getNumRecords(VM& vm, Thread& thread) {
    auto& record_store_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& record_store_native = std::get<RecordStoreNativeData>(record_store_instance.native_payload);
    int32_t num_records = static_cast<int32_t>(record_store_native.record_store->size());
    thread.current_frame().push_stack(num_records);
}

void javax_microedition_rms_RecordStore_openRecordStore(VM& vm, Thread& thread) {
    auto create_if_necessary = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& name_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& name_native = std::get<StringNativeData>(name_instance.native_payload);

    auto* record_store_obj = vm.heap().new_instance(vm.class_loader().load("javax/microedition/rms/RecordStore"));
    auto& record_store_instance = std::get<InstanceData>(record_store_obj->data);
    
    auto* record_store = vm.record_store_manager().open_record_store(name_native.value, create_if_necessary != 0);
    if (record_store == nullptr) {
        Class& exc_class = vm.class_loader().load("javax/microedition/rms/RecordStoreNotFoundException");
        Object* exc = vm.heap().new_instance(exc_class);
        // TODO(Kostu): initialize exception with message
        thread.set_pending_exception(exc);
        return;
    }
    
    RecordStoreNativeData native_data{
        .record_store = record_store
    };
    record_store_instance.native_payload = native_data;

    thread.current_frame().push_stack(record_store_obj);
}

void javax_microedition_rms_RecordStore_setRecord(VM& vm, Thread& thread) {
    auto size = std::get<int32_t>(thread.current_frame().pop_stack());
    auto offset = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& data_array = std::get<PrimitiveArrayData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto record_id = std::get<int32_t>(thread.current_frame().pop_stack());
    auto& record_store_instance = std::get<InstanceData>(std::get<Object*>(thread.current_frame().pop_stack())->data);
    auto& record_store_native = std::get<RecordStoreNativeData>(record_store_instance.native_payload);

    if (offset < 0 || size < 0 || offset + size > data_array.length()) {
        // In a complete VM this would raise ArrayIndexOutOfBoundsException.
        throw std::runtime_error("RecordStore.addRecord: offset and size out of bounds");
    }

    auto& elements = std::get<std::vector<uint8_t>>(data_array.elements);
    record_store_native.record_store->set_record(
        record_id, std::span<const uint8_t>(elements.data() + offset, size));
}

}

NativeMethods::NativeMethods() {
    storage_.insert({
        { "com/kostu96/gjvm/io/ResourceInputStream.init(Ljava/lang/String;)V",
           com_kostu96_gjvm_io_ResourceInputStream_init },
        { "com/kostu96/gjvm/io/ResourceInputStream.read()I",
           com_kostu96_gjvm_io_ResourceInputStream_read },

        { "java/lang/Class.forName(Ljava/lang/String;)Ljava/lang/Class;", java_lang_Class_forName },
        { "java/lang/Class.getName()Ljava/lang/String;", java_lang_Class_getName },
        { "java/lang/Object.getClass()Ljava/lang/Class;", java_lang_Object_getClass },
        { "java/lang/String.charAt(I)C", java_lang_String_charAt },
        { "java/lang/String.indexOf(II)I", java_lang_String_indexOf },
        { "java/lang/String.init([BII)V", java_lang_String_init_byte_array },
        { "java/lang/String.init(Ljava/lang/StringBuffer;)V", java_lang_String_init_StringBuffer },
        { "java/lang/String.init([CII)V", java_lang_String_init_char_array },
        { "java/lang/String.lastIndexOf(I)I", java_lang_String_lastIndexOf },
        { "java/lang/String.replace(CC)Ljava/lang/String;", java_lang_String_replace },
        { "java/lang/String.substringNative(II)Ljava/lang/String;", java_lang_String_substringNative },
        { "java/lang/String.toUpperCase()Ljava/lang/String;", java_lang_String_toUpperCase },
        { "java/lang/StringBuffer.append(Ljava/lang/String;)Ljava/lang/StringBuffer;", java_lang_StringBuffer_append },
        { "java/lang/StringBuffer.init()V", java_lang_StringBuffer_init },
        { "java/lang/System.arraycopy(Ljava/lang/Object;ILjava/lang/Object;II)V", java_lang_System_arraycopy },
        { "java/lang/System.currentTimeMillis()J", java_lang_System_currentTimeMillis },
        { "java/lang/System.getProperty(Ljava/lang/String;)Ljava/lang/String;", java_lang_System_getProperty },
        { "java/lang/Thread.start()V", java_lang_Thread_start },
        
        { "javax/microedition/lcdui/Canvas.getHeight()I", javax_microedition_lcdui_Canvas_getHeight },
        { "javax/microedition/lcdui/Canvas.getWidth()I", javax_microedition_lcdui_Canvas_getWidth },
        { "javax/microedition/lcdui/Canvas.repaint(IIII)V", javax_microedition_lcdui_Canvas_repaint },
        { "javax/microedition/lcdui/Canvas.serviceRepaints()V", javax_microedition_lcdui_Canvas_serviceRepaints },
        { "javax/microedition/lcdui/Font.init()V", javax_microedition_lcdui_Font_init },
        { "javax/microedition/lcdui/Graphics.drawStringNative(Ljava/lang/String;II)V",
           javax_microedition_lcdui_Graphics_drawStringNative },
        { "javax/microedition/lcdui/Graphics.fillRect(IIII)V", javax_microedition_lcdui_Graphics_fillRect },
        { "javax/microedition/lcdui/Graphics.init(Ljavax/microedition/lcdui/Image;)V",
           javax_microedition_lcdui_Graphics_init },
        { "javax/microedition/lcdui/Image.createImage([BII)Ljavax/microedition/lcdui/Image;",
           javax_microedition_lcdui_Image_createImage_byte_array },
        { "javax/microedition/lcdui/Image.createImage(Ljava/io/InputStream;)Ljavax/microedition/lcdui/Image;",
           javax_microedition_lcdui_Image_createImage_InputStream },
        { "javax/microedition/lcdui/Image.getHeight()I", javax_microedition_lcdui_Image_getHeight },
        { "javax/microedition/lcdui/Image.getRGB([IIIIIII)V", javax_microedition_lcdui_Image_getRGB },
        { "javax/microedition/lcdui/Image.getWidth()I", javax_microedition_lcdui_Image_getWidth },
        { "javax/microedition/lcdui/Image.init(II)V", javax_microedition_lcdui_Image_init },
        { "javax/microedition/rms/RecordStore.addRecord([BII)I", javax_microedition_rms_RecordStore_addRecord },
        { "javax/microedition/rms/RecordStore.getNumRecords()I", javax_microedition_rms_RecordStore_getNumRecords },
        { "javax/microedition/rms/RecordStore.openRecordStore(Ljava/lang/String;Z)Ljavax/microedition/rms/RecordStore;",
           javax_microedition_rms_RecordStore_openRecordStore },
        { "javax/microedition/rms/RecordStore.setRecord(I[BII)V", javax_microedition_rms_RecordStore_setRecord }
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
