#pragma once
#include <functional>
#include <string>
#include <vector>

class VM;
class Frame;

struct NativeMethod {
    std::string name;
    std::string descriptor;
    std::function<void(VM&, Frame&)> callback;
};

struct NativeClassDescription {
    std::string name;
    std::string super_name = "java/lang/Object";
    std::vector<NativeMethod> methods;

    static NativeClassDescription data_input_stream();
    static NativeClassDescription object();
    static NativeClassDescription clazz();
    static NativeClassDescription hashtable();
    static NativeClassDescription random();
    static NativeClassDescription stack();
    static NativeClassDescription string();
    static NativeClassDescription vector();

    static NativeClassDescription midlet();

    static NativeClassDescription full_canvas();
};
