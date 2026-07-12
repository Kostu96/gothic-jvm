#pragma once
#include <functional>
#include <string>
#include <unordered_map>

class Frame;
class VM;

class NativeMethods {
public:
    using Callback = std::function<void(VM&, Frame&)>;

    NativeMethods();

    const Callback* find(std::string_view class_name,
                         std::string_view name,
                         std::string_view descriptor) const;
private:
    std::unordered_map<std::string, Callback> storage_;
};
