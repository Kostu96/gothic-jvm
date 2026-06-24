#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class Class;

// Resolves binary class names ("java/lang/Object") to loaded Class instances
// by searching the configured classpath entries. Owns every loaded Class.
class ClassLoader {
public:
    ClassLoader();
    ~ClassLoader();

    void add_classpath_entry(std::filesystem::path dir);

    // Loads and returns the class. Throws if the file cannot be found or parsed.
    // Returns the same pointer on repeated calls with the same name.
    Class* load(std::string_view binary_name);

    Class* find_loaded(std::string_view binary_name) const noexcept;

    ClassLoader(const ClassLoader&) = delete;
    ClassLoader& operator=(const ClassLoader&) = delete;
private:
    std::filesystem::path resolve_path(std::string_view binary_name) const;

    std::vector<std::filesystem::path> classpath_;
    std::unordered_map<std::string, std::unique_ptr<Class>> loaded_;
};
