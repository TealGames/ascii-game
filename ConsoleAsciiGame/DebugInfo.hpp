#pragma once
#include <string>
#include <vector>
#include "Vec2.hpp"
#include "WorldPosition.hpp"

class DebugInfo
{
private:
	std::vector<std::string> m_text;
	std::vector<std::size_t> m_highlightedIndices;

public:

private:
public:
	DebugInfo();

	void ClearProperties();
	void AddProperty(const std::string& name, const std::string& value);
	const std::vector<std::string>& GetText() const;

	bool TryAddHighlightedIndex(const size_t& index);
	void RemoveHighlightedIndex(const size_t& index);
	void ClearHighlightedIndices();
	const std::vector<std::size_t>& GetHighlightedIndicesSorted() const;
};

