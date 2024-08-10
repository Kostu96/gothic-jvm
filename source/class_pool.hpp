#pragma once
#include "types.hpp"
#include "class.hpp"

#include <filesystem>
#include <unordered_map>
#include <string>
#include <vector>

class ClassPool
{
public:
	ClassPool() = default;

	void setClassPath(const std::filesystem::path& path) { m_classPath = path; }

	u32 loadClass(const char* name);
	u32 resolveClass(const char* name);

	Class& getClass(u32 index) { return m_classes[index]; }
private:
	std::filesystem::path m_classPath;
	std::vector<Class> m_classes;
	std::unordered_map<std::string, u32> m_classNameToIndexMap;
};
