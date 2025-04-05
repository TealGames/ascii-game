#include "pch.hpp"
#include "Asset.hpp"
#include "Debug.hpp"

Asset::Asset(const std::filesystem::path& path) : m_name(), m_path(path), m_dependenciesSet(false)
{
	if (!Assert(std::filesystem::exists(m_path), std::format("Tried to create an asset at path: {} "
		"but that path does not exist", m_path.string())))
		return;

	if (!Assert(m_path.has_filename(), std::format("Tried to create an asset at path: {} "
		"but that path does not lead to a file", m_path.string())))
		return;

	m_name = ExtractNameFromFile(m_path);
}

std::string Asset::ExtractNameFromFile(const std::filesystem::path& path)
{
	return path.stem().string();
}

std::string Asset::GetName() const
{
	return m_name;
}
std::filesystem::path Asset::GetPath() const
{
	return m_path;
}
bool Asset::AreDependenciesSet() const
{
	return m_dependenciesSet;
}
void Asset::MarkDependenciesSet()
{
	m_dependenciesSet = true;
}

std::string Asset::ToString() const
{
	return std::format("[Asset:'{}' @path:{}]", GetName(), GetPath().string());
}