#include "class_loader/class_loader.hpp"

#include "class_loader/native_class_description.hpp"
#include "runtime/class.hpp"

#include <stdexcept>

namespace {

std::unordered_map<std::string, NativeClassDescription> native_classes_desc = {
    { "java/io/DataInputStream", NativeClassDescription::data_input_stream() },
    { "java/lang/Object", NativeClassDescription::object() },
    { "java/lang/Class", NativeClassDescription::clazz() },
    { "java/lang/String", NativeClassDescription::string() },
    { "java/util/Hashtable", NativeClassDescription::hashtable() },
    { "java/util/Random", NativeClassDescription::random() },
    { "java/util/Stack", NativeClassDescription::stack() },
    { "java/util/Vector", NativeClassDescription::vector() },
    { "javax/microedition/midlet/MIDlet", NativeClassDescription::midlet() },
    { "com/nokia/mid/ui/FullCanvas", NativeClassDescription::full_canvas() }
};

}

ClassLoader::ClassLoader() = default;
ClassLoader::~ClassLoader() = default;

void ClassLoader::add_classpath_entry(std::filesystem::path dir) {
    classpath_.push_back(std::move(dir));
}

Class* ClassLoader::load_native(std::string_view binary_name) {
    if (Class* existing = find_loaded(binary_name)) {
        return existing;
    }

    auto it = native_classes_desc.find(std::string(binary_name));
    if (it == native_classes_desc.end()) {
        throw std::runtime_error(
            "ClassLoader: cannot find native class '" + std::string(binary_name) + "'");
    }

    auto cls = std::make_unique<Class>(it->second);
    Class* raw = cls.get();
    loaded_.emplace(std::string(binary_name), std::move(cls));
    return raw;
}

Class* ClassLoader::load(std::string_view binary_name) {
    if (Class* existing = find_loaded(binary_name)) {
        return existing;
    }

    if (!binary_name.empty() && binary_name.front() == '[') {
        return load_array(binary_name);
    }

    std::filesystem::path file = resolve_path(binary_name);
    if (file.empty()) {
        throw std::runtime_error(
            "ClassLoader: cannot find class '" + std::string(binary_name) + "' on classpath");
    }

    auto cls = std::make_unique<Class>(file.string().c_str());
    Class* raw = cls.get();
    loaded_.emplace(std::string(binary_name), std::move(cls));
    return raw;
}

Class* ClassLoader::load_array(std::string_view array_name) {
    std::string_view component = array_name.substr(1); // strip one '['

    Class* element = nullptr;
    if (!component.empty()) {
        if (component.front() == '[') {
            element = load(component); // nested array class
        }
        else if (component.size() >= 2 && component.front() == 'L' && component.back() == ';') {
            element = load(component.substr(1, component.size() - 2)); // L...; -> binary name
        }
        // else: primitive base element (e.g. "[I"); no Class is modeled, leave null
    }

    auto cls = std::make_unique<Class>(std::string(array_name), element);
    Class* raw = cls.get();
    loaded_.emplace(std::string(array_name), std::move(cls));
    return raw;
}

Class* ClassLoader::find_loaded(std::string_view binary_name) const noexcept {
    auto it = loaded_.find(std::string(binary_name));
    return it != loaded_.end() ? it->second.get() : nullptr;
}

std::filesystem::path ClassLoader::resolve_path(std::string_view binary_name) const {
    std::filesystem::path relative = std::filesystem::path(binary_name).concat(".class");
    for (const auto& entry : classpath_) {
        std::filesystem::path candidate = entry / relative;
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }
    return {};
}
