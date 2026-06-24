#include "runtime/class_loader.hpp"

#include "runtime/class.hpp"

#include <stdexcept>

// Defined out-of-line so the std::unique_ptr<Class> destructors in `loaded_`
// only need to be instantiated in this TU, where Class is a complete type.
ClassLoader::ClassLoader() = default;
ClassLoader::~ClassLoader() = default;

void ClassLoader::add_classpath_entry(std::filesystem::path dir) {
    classpath_.push_back(std::move(dir));
}

Class* ClassLoader::find_loaded(std::string_view binary_name) const noexcept {
    auto it = loaded_.find(std::string(binary_name));
    return it != loaded_.end() ? it->second.get() : nullptr;
}

Class* ClassLoader::load(std::string_view binary_name) {
    if (Class* existing = find_loaded(binary_name)) {
        return existing;
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
