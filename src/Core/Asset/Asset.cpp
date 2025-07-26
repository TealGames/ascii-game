#include "pch.hpp"
#include "Core/Asset/Asset.hpp"
#include "Core/Analyzation/Debug.hpp"
#include <fstream>

Asset::Asset(const std::filesystem::path& path, const bool hasDependencies) 
//Note: if we do not have dependencies we can say they are already set
	: m_name(), m_absolutePath(path), m_dependenciesSet(!hasDependencies)
{
	if (path.empty()) return;

	if (!Assert(std::filesystem::exists(m_absolutePath), std::format("Tried to create an asset at path: {} "
		"but that path does not exist", m_absolutePath.string())))
		return;

	if (!Assert(m_absolutePath.has_filename(), std::format("Tried to create an asset at path: {} "
		"but that path does not lead to a file", m_absolutePath.string())))
		return;

	m_name = ExtractNameFromFile(m_absolutePath);
}

std::string Asset::ExtractNameFromFile(const std::filesystem::path& path)
{
	return path.stem().string();
}

const std::string& Asset::GetName() const
{
	return m_name;
}
void Asset::OverrideAssetName(const std::string& name)
{
	m_name = name;
}

std::filesystem::path Asset::GetPathCopy() const
{
	return m_absolutePath;
}
const std::filesystem::path& Asset::GetPath() const
{
	return m_absolutePath;
}

bool Asset::AreDependenciesSet() const
{
	return m_dependenciesSet;
}
void Asset::MarkDependenciesSet()
{
	m_dependenciesSet = true;
}

void Asset::SaveToSelf()
{
	SaveToPath(GetPathCopy());
}
void Asset::SaveToPath(const std::filesystem::path& path)
{
	return;
}

std::string Asset::ToString() const
{
	return std::format("[Asset:'{}' @path:{}]", GetName(), GetPathCopy().string());
}