#include "pch.hpp"
#include "DebugInfo.hpp"

DebugInfo::DebugInfo() : m_text() {}

void DebugInfo::ClearProperties()
{
	m_text.clear();
}
void DebugInfo::AddProperty(const std::string& name, const std::string& value)
{
	std::string fullStr = std::format("{}: {}", name, value);
	m_text.push_back(fullStr.c_str());
}

const std::vector<std::string>& DebugInfo::GetText() const
{
	return m_text;
}
