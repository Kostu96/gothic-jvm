#include "class_pool.hpp"
#include "class_file.hpp"
#include "class.hpp"

ClassPool::~ClassPool()
{
	for (auto& c : m_classes)
		delete c.second;
}

void ClassPool::addToClassPath(const std::filesystem::path& path)
{
	m_classPath.push_back(path);
}

Class* ClassPool::loadClass(VM& vm, std::string_view name)
{
	if (auto search = m_classes.find(std::string(name)); search != m_classes.end())
	{
		return search->second;
	}
	else
	{
		std::string filepath;
		for (std::filesystem::path path : m_classPath)
		{
			path /= name;
			path += ".class";

			if (std::filesystem::exists(path)) {
				filepath = path.string();
				break;
			}
		}
		assert(!filepath.empty());

		Class* c = new Class(vm, filepath.c_str());
		m_classes.emplace(name, c);
		return c;
	}
}
