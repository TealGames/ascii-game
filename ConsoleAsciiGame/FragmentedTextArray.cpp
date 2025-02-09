#include "pch.hpp"
#include "FragmentedTextArray.hpp"
#include "RaylibUtils.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"

std::string ToString(const TextArrayPositionType& positionType)
{
	if (positionType == TextArrayPositionType::ArrayIndex) return "ArrayIndex";
	else if (positionType == TextArrayPositionType::PositionData) return "PositionData";
	else return "[TEXT ARRAY POSITION TYPE ENUM PARSE FAILED]";
}

FragmentedTextArray::FragmentedTextArray() : FragmentedTextArray(std::vector<std::vector<TextCharPosition>>{}) {}

FragmentedTextArray::FragmentedTextArray(const std::vector<std::vector<TextCharPosition>>& chars) 
	: m_textArray(chars), m_minIndices(NULL_INDEX, NULL_INDEX), m_maxIndices(NULL_INDEX, NULL_INDEX)
{
	for (int r = 0; r < m_textArray.size(); r++)
	{
		for (int c = 0; c < m_textArray[r].size(); c++)
		{
			const Array2DPosition& currentPos = m_textArray[r][c].m_RowColPos;
			if (currentPos.GetRow() < m_minIndices.GetRow()) m_minIndices.SetRow(r);
			if (currentPos.GetRow() > m_maxIndices.GetRow()) m_maxIndices.SetRow(r);

			if (currentPos.GetCol() < m_minIndices.GetCol()) m_minIndices.SetCol(c);
			if (currentPos.GetCol() > m_maxIndices.GetCol()) m_maxIndices.SetCol(c);
		}
	}
}

TextCharPosition* FragmentedTextArray::TryGetPosMutable(const Array2DPosition& rowColPos, const TextArrayPositionType& positionType)
{
	//TODO: run benchamarks to see if checking min and max is actually worth it?

	if (positionType==TextArrayPositionType::ArrayIndex)
	{
		if (Assert(this, IsValidIndexPos(rowColPos), std::format("Tried to get pos MUTABLE from row col index: {} "
			"but it is out of bounds", rowColPos.ToString()))) return nullptr;
		return &(m_textArray[rowColPos.GetRow()][rowColPos.GetCol()]);
	}

	const int& r = rowColPos.GetRow();
	const int& c = rowColPos.GetCol();
	if (r > m_maxIndices.GetRow()) return nullptr;
	if (r < m_minIndices.GetRow()) return nullptr;
	if (c > m_maxIndices.GetCol()) return nullptr;
	if (c < m_minIndices.GetCol()) return nullptr;

	if (r == m_minIndices.GetRow() && c == m_minIndices.GetCol()) return &(m_textArray[m_minIndices.GetRow()][m_minIndices.GetCol()]);
	if (r == m_minIndices.GetRow() && c == m_maxIndices.GetCol()) return &(m_textArray[m_minIndices.GetRow()][m_maxIndices.GetCol()]);
	if (r == m_maxIndices.GetRow() && c == m_minIndices.GetCol()) return &(m_textArray[m_maxIndices.GetRow()][m_minIndices.GetCol()]);
	if (r == m_maxIndices.GetRow() && c == m_maxIndices.GetCol()) return &(m_textArray[m_maxIndices.GetRow()][m_maxIndices.GetCol()]);

	for (int row = 0; row < m_textArray.size(); row++)
	{
		for (int col = 0; col < m_textArray[r].size(); col++)
		{
			if (m_textArray[r][c].m_RowColPos == rowColPos) return &(m_textArray[r][c]);
		}
	}
	return nullptr;
}

const TextCharPosition* FragmentedTextArray::TryGetPos(const Array2DPosition& rowColPos, const TextArrayPositionType& positionType)
{
	return TryGetPosMutable(rowColPos, positionType);
}

bool FragmentedTextArray::DoesPositionExist(const Array2DPosition& rowColPos)
{
	return TryGetPos(rowColPos, TextArrayPositionType::PositionData) != nullptr;
}
bool FragmentedTextArray::IsValidIndexPos(const Array2DPosition& rowCol)
{
	return 0 <= rowCol.GetRow() && rowCol.GetRow() < m_textArray.size() &&
		0 <= rowCol.GetCol() && rowCol.GetCol() < m_textArray[rowCol.GetRow()].size();
}

void FragmentedTextArray::SetAt(const Array2DPosition& rowColPos, const TextArrayPositionType& positionType, const TextChar& newBufferChar)
{
	TextCharPosition* pos = TryGetPosMutable(rowColPos, positionType);
	if (!Assert(this, pos != nullptr, std::format("Tried to set position: {}({}) with BUFFER CHAR: {} for fragmented text array "
		"but data was not found", rowColPos.ToString(), ::ToString(positionType), newBufferChar.ToString()))) return;

	pos->m_Text = newBufferChar;
}
void FragmentedTextArray::SetAt(const Array2DPosition& rowColPos, const TextArrayPositionType& positionType, const char& newChar)
{
	TextCharPosition* pos = TryGetPosMutable(rowColPos, positionType);
	if (!Assert(this, pos != nullptr, std::format("Tried to set position: {}({}) with CHAR: {} for fragmented text array "
		"but data was not found", rowColPos.ToString(), ::ToString(positionType), Utils::ToString(newChar)))) return;

	pos->m_Text.m_Char = newChar;
}
void FragmentedTextArray::SetAt(const Array2DPosition& rowColPos, const TextArrayPositionType& positionType, const Color& newColor)
{
	TextCharPosition* pos = TryGetPosMutable(rowColPos, positionType);
	if (!Assert(this, pos != nullptr, std::format("Tried to set position: {}({}) with COLOR: {} for fragmented text array "
		"but data was not found", rowColPos.ToString(), ::ToString(positionType), RaylibUtils::ToString(newColor)))) return;

	pos->m_Text.m_Color = newColor;
}

void FragmentedTextArray::SetAt(const std::vector<Array2DPosition>& rowColPos, const TextArrayPositionType& positionType, const TextChar& newBufferChar)
{
	TextCharPosition* dataPtr = nullptr;

	//TODO: this should be more optimized for positions data searching since it is O(n^2) for every search
	for (const auto& pos : rowColPos)
	{
		dataPtr = TryGetPosMutable(pos, positionType);
		if (!Assert(this, dataPtr != nullptr, std::format("Tried to set position: {}({}) OF OTHER POSITIONS with buffer: {} for fragmented text array "
			"but data was not found", pos.ToString(), ::ToString(positionType), newBufferChar.ToString()))) return;

		dataPtr->m_Text = newBufferChar;
	}
}
void FragmentedTextArray::SetAt(const std::vector<TextCharPosition>& updatedCharsAtPos, const TextArrayPositionType& positionType)
{
	TextCharPosition* dataPtr = nullptr;

	//TODO: this should be more optimized for positions data searching since it is O(n^2) for every search
	for (const auto& updatedChars : updatedCharsAtPos)
	{
		dataPtr = TryGetPosMutable(updatedChars.m_RowColPos, positionType);
		if (!Assert(this, dataPtr != nullptr, std::format("Tried to set position: {}({}) OF OTHER POSITIONS with text buffer: {} for fragmented text array "
			"but data was not found", updatedChars.m_RowColPos.ToString(), ::ToString(positionType), updatedChars.ToString()))) return;

		dataPtr->m_Text = updatedChars.m_Text;
	}
}
void FragmentedTextArray::SetAt(const std::vector<ColorPosition>& updateColorsAtPos, const TextArrayPositionType& positionType)
{
	TextCharPosition* dataPtr = nullptr;

	//TODO: this should be more optimized for positions data searching since it is O(n^2) for every search
	for (const auto& updatedChars : updateColorsAtPos)
	{
		dataPtr = TryGetPosMutable(updatedChars.m_RowColPos, positionType);
		if (!Assert(this, dataPtr != nullptr, std::format("Tried to set position: {}({}) OF OTHER POSITIONS with text color: {} for fragmented text array "
			"but data was not found", updatedChars.m_RowColPos.ToString(), ::ToString(positionType), updatedChars.ToString()))) return;

		dataPtr->m_Text.m_Color = updatedChars.m_Color;
	}
}

std::string FragmentedTextArray::ToString(const std::vector<std::vector<TextCharPosition>>& chars,
	const bool convertAll)
{
	std::string fullStr = "";
	char currentChar = '0';
	for (int r = 0; r < chars.size(); r++)
	{
		for (int c = 0; c < chars[r].size(); c++)
		{
			currentChar = chars[r][c].m_Text.m_Char;
			fullStr += std::format("{}({})", currentChar, chars[r][c].m_RowColPos.ToString());

			if (convertAll)
			{
				fullStr += RaylibUtils::ToString(chars[r][c].m_Text.m_Color) + " ";
			}
		}
		fullStr += "\n";
	}
	return fullStr;
}

std::string FragmentedTextArray::ToString(bool convertChars) const
{
	return ToString(m_textArray, convertChars);
}

FragmentedTextArray& FragmentedTextArray::operator=(const FragmentedTextArray& other)
{
	//Log("USING TEXT BUFFER LVALUE = OPERATOR");
	if (&other == this) return *this;

	//m_TextArray = other.m_TextArray;
	m_textArray = other.m_textArray;
	return *this;
}

FragmentedTextArray& FragmentedTextArray::operator=(FragmentedTextArray&& other) noexcept
{
	//Log("USING TEXT BUFFER RVALUE = OPERATOR");
	if (this == &other)
		return *this;

	m_textArray = std::exchange(other.m_textArray, {});

	return *this;
}