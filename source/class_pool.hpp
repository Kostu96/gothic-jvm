#pragma once
#include "common.hpp"

#include <filesystem>

class Class;

class ClassPool
{
public:
	ClassPool() = default;
	~ClassPool();

	void addToClassPath(const std::filesystem::path& path);

	u32 loadClass(std::string_view name);

	Class& getClass(u32 index) { return *m_classes[index]; }
private:
	std::vector<std::filesystem::path> m_classPath;
	std::vector<Class*> m_classes;
	std::unordered_map<std::string, u32> m_classNameToIndexMap;
};
