#include "pch.hpp"
#include "DebugInfo.hpp"

DebugInfo::DebugInfo() : 
	m_text(), m_highlightedIndices(), m_mouseDebugData(std::nullopt) {}

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

bool DebugInfo::TryAddHighlightedIndex(const size_t& index)
{
	if (std::find(m_highlightedIndices.begin(), m_highlightedIndices.end(), index) !=
		m_highlightedIndices.end()) return false;

	m_highlightedIndices.push_back(index);
	//TODO: yes this is inneficient but we want to make sure get function does not mutate the data
	std::sort(m_highlightedIndices.begin(), m_highlightedIndices.end());
	return true;
}
void DebugInfo::RemoveHighlightedIndex(const size_t& index)
{
	auto it = std::find(m_highlightedIndices.begin(), m_highlightedIndices.end(), index);
	if (it == m_highlightedIndices.end()) return;

	m_highlightedIndices.erase(it);
}
void DebugInfo::ClearHighlightedIndices()
{
	m_highlightedIndices.clear();
}
const std::vector<std::size_t>& DebugInfo::GetHighlightedIndicesSorted() const
{
	return m_highlightedIndices;
}

void DebugInfo::SetMouseDebugData(const DebugMousePosition& info)
{
	m_mouseDebugData = info;
}
const std::optional<DebugMousePosition>& DebugInfo::GetMouseDebugData() const
{
	return m_mouseDebugData;
}