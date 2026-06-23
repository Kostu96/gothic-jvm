#pragma once
#include "common.hpp"

#include <filesystem>

class Class;
class VM;

class ClassPool
{
public:
	ClassPool() = default;
	~ClassPool();

	void addToClassPath(const std::filesystem::path& path);

	Class* loadClass(VM& vm, std::string_view name);
private:
	std::vector<std::filesystem::path> m_classPath;
	std::unordered_map<std::string, Class*> m_classes;
};
