#pragma once
#include <vector>
#include "TextArray.hpp"
#include "FragmentedTextArray.hpp"

using VisualData = TextArray;
//using VisualDataPositions = FragmentedTextArray;

//struct VisualData
//{
//	//TODO: should sprites have emptyu char for locations not covered or should they store the position?
//	std::vector<std::vector<TextChar>> m_Data = {};
//};
//
struct VisualDataPositions
{
	//TODO: should sprites have emptyu char for locations not covered or should they store the position?
	std::vector<TextCharPosition> m_Data = {};
};
