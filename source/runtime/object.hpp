#pragma once
#include "runtime/class.hpp"
#include "runtime/value.hpp"

#include <cstdint>
#include <variant>
#include <vector>

class Class;
struct SDL_Surface;
struct SDL_Renderer;

struct ResourceInputStreamNativeData {
    std::vector<uint8_t> buffer;
    size_t position;
};

struct StringNativeData {
    std::string value;
};

struct ImageNativeData {
    SDL_Surface* sdl_surface = nullptr;
};

struct GraphicsNativeData {
    SDL_Renderer* sdl_renderer = nullptr;
    SDL_Surface* sdl_surface = nullptr;
};

using NativePayload = std::variant<std::monostate,
                                   ResourceInputStreamNativeData,
                                   StringNativeData,
                                   ImageNativeData,
                                   GraphicsNativeData>;

struct InstanceData {
    Class& type;
    std::vector<Value> fields;
    NativePayload native_payload;
};

struct PrimitiveArrayData {
    enum class ElementType : uint8_t {
        Boolean = 4,
        Char = 5,
        Float = 6,
        Double = 7,
        Byte = 8,
        Short = 9,
        Int = 10,
        Long = 11,
    };

    PrimitiveArrayData(ElementType type, int32_t length);

    std::variant<
        std::vector<uint8_t>,  // boolean, byte
        std::vector<char16_t>, // char
        std::vector<int16_t>,  // short
        std::vector<int32_t>,  // int
        std::vector<int64_t>,  // long
        std::vector<float>,    // float
        std::vector<double>    // double
    > elements;

    int32_t length() const noexcept;
    Value get(int32_t index) const;
    void set(int32_t index, Value value);
};

struct InstanceArrayData {
    InstanceArrayData(Class& type, int32_t length);

    Class& element_type;

    std::vector<Object*> elements;
};

struct ClassMirrorData {
    Class& mirrored;
};

struct Object {
    std::variant<InstanceData, PrimitiveArrayData, InstanceArrayData, ClassMirrorData> data;
    struct Monitor {
        Thread* owner = nullptr;
        uint32_t recursion_count = 0;
    } monitor;
};
