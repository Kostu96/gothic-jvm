#include "class_pool.hpp"
#include "class_file.hpp"
#include "class.hpp"

ClassPool::~ClassPool()
{
	for (auto& c : m_classes)
		delete c;
}

void ClassPool::addToClassPath(const std::filesystem::path& path)
{
	m_classPath.push_back(path);
}

u32 ClassPool::loadClass(std::string_view name)
{
	if (auto search = m_classNameToIndexMap.find(std::string(name)); search != m_classNameToIndexMap.end())
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

		Class* c = new Class(*this, filepath.c_str());

		m_classes.emplace_back(c);
		u32 index = (u32)m_classes.size() - 1;
		m_classNameToIndexMap.emplace(name, index);

		return index;
	}
}
