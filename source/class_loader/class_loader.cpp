#include "class_loader/class_loader.hpp"

#include "runtime/class.hpp"

#include <print>
#include <stdexcept>

namespace {

std::string_view primitive_type_name(char descriptor) {
    switch (descriptor) {
    case 'B': return "byte";
    case 'C': return "char";
    case 'D': return "double";
    case 'F': return "float";
    case 'I': return "int";
    case 'J': return "long";
    case 'S': return "short";
    case 'Z': return "boolean";
    default:
        throw std::runtime_error(
            std::string("ClassLoader: invalid array component descriptor '") + descriptor + "'");
    }
}

} // namespace

ClassLoader::ClassLoader() = default;
ClassLoader::~ClassLoader() = default;

void ClassLoader::add_classpath_entry(std::filesystem::path dir) {
    classpath_.push_back(std::move(dir));
}

Class& ClassLoader::load(std::string_view binary_name) {
    if (auto it = loaded_.find(std::string(binary_name)); it != loaded_.end()) {
        return *it->second;
    }

    if (!binary_name.empty() && binary_name.front() == '[') {
        return load_array(binary_name);
    }

    std::filesystem::path file = resolve_path(binary_name);
    if (file.empty()) {
        throw std::runtime_error(
            "ClassLoader: cannot find class '" + std::string(binary_name) + "' on classpath");
    }

    std::println("Loading class: {}", binary_name);
    auto cls = std::make_unique<Class>(file.string().c_str(), *this);
    if (cls->this_name() != binary_name) {
        throw std::runtime_error(
            "ClassLoader: class name mismatch for '" + std::string(binary_name) +
            "'; expected '" + std::string(cls->this_name()) + "'");
    }

    Class& raw = *cls;
    loaded_.emplace(std::string(binary_name), std::move(cls));
    return raw;
}

Class& ClassLoader::load_array(std::string_view array_name) {
    // array_name is guaranteed to start with '[' (checked by load()).
    std::string_view component_descriptor = array_name.substr(1);
    if (component_descriptor.empty()) {
        throw std::runtime_error(
            "ClassLoader: malformed array class name '" + std::string(array_name) + "'");
    }

    Class* component_type = nullptr;
    switch (component_descriptor.front()) {
    case '[': // nested array: recurse to load the inner synthetic array class
        component_type = &load(component_descriptor);
        break;
    case 'L': // object reference: L<binary_name>;
        component_type = &load(component_descriptor.substr(1, component_descriptor.size() - 2));
        break;
    default: { // primitive: synthesize a class with a null component type
        std::string name(primitive_type_name(component_descriptor.front()));
        auto it = loaded_.find(name);
        if (it == loaded_.end()) {
            it = loaded_.emplace(name, std::make_unique<Class>(name)).first;
        }
        component_type = it->second.get();
        break;
    }
    }

    auto cls = std::make_unique<Class>(std::string(array_name), component_type);
    Class& raw = *cls;
    loaded_.emplace(std::string(array_name), std::move(cls));
    return raw;
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
