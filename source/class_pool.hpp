#pragma once
#include "types.hpp"
#include "class.hpp"

#include <filesystem>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>

class ClassPool
{
public:
	ClassPool() = default;

	void addToClassPath(const std::filesystem::path& path);

	u32 loadClass(std::string_view name);

	Class& getClass(u32 index) { return m_classes[index]; }
private:
	std::vector<std::filesystem::path> m_classPath;
	std::vector<Class> m_classes;
	std::unordered_map<std::string, u32> m_classNameToIndexMap;
};
