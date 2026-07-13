#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class Class;
class NativeMethods;

class ClassLoader {
public:
    explicit ClassLoader(const NativeMethods& native_methods);
    ~ClassLoader();

    void add_classpath_entry(std::filesystem::path dir);

    Class& load(std::string_view binary_name);

    std::vector<uint8_t> load_resource(std::string_view binary_name);

    const NativeMethods& native_methods() const noexcept { return native_methods_; }

    ClassLoader(const ClassLoader&) = delete;
    ClassLoader& operator=(const ClassLoader&) = delete;
private:
    std::filesystem::path resolve_path(std::string_view binary_name, bool is_class = true) const;

    Class& load_array(std::string_view array_name);

    std::vector<std::filesystem::path> classpath_;
    std::unordered_map<std::string, std::unique_ptr<Class>> loaded_;
    const NativeMethods& native_methods_;
};
