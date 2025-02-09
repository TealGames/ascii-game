#include "pch.hpp"
#include "raylib.h"
#include "TextArray.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"
#include "Array2DPosition.hpp"
#include "Point2DInt.hpp"
#include "Debug.hpp"

TextChar::TextChar() :
	m_Color{}, m_Char(EMPTY_CHAR_PLACEHOLDER) {}

TextChar::TextChar(const Color& color, const char& textChar = EMPTY_CHAR_PLACEHOLDER) :
	m_Color(color), m_Char(textChar) {}

bool TextChar::operator==(const TextChar& other) const
{
	return RaylibUtils::ColorEqual(m_Color, other.m_Color) && m_Char == other.m_Char;
}

std::string TextChar::ToString() const
{
	return std::format("[Color: {} C:{}]",
		RaylibUtils::ToString(m_Color), Utils::ToString(m_Char));
}

TextCharPosition::TextCharPosition(const Array2DPosition& pos, const TextChar& textChar)
	: m_RowColPos(pos), m_Text(textChar)
{

}

std::string TextCharPosition::ToString() const
{
	return std::format("[Pos:{}, Data:{}]",
		m_RowColPos.ToString(), m_Text.ToString());
}

ColorPosition::ColorPosition(const Array2DPosition& pos, const Color& color) :
	m_RowColPos(pos), m_Color(color) {}

std::string ColorPosition::ToString() const
{
	return std::format("[Pos:{}, Color:{}]",
		m_RowColPos.ToString(), RaylibUtils::ToString(m_Color));
}

TextArray::TextArray() :
	TextArray(0, 0, std::vector<std::vector<TextChar>>{ {TextChar(Color(), EMPTY_CHAR_PLACEHOLDER)} }) {}

TextArray::TextArray(const int& width, const int& height, const std::vector<std::vector<TextChar>>& chars) :
	m_width(width), m_height(height), m_TextArray(chars)
{
	if (m_height == 0) return;
	if (!Assert(this, chars.size() == m_height, std::format("Tried to init a text array with height ({}) "
		"that does not match character arg ({})!",
		std::to_string(chars.size()), std::to_string(m_height)))) return;

	for (auto& textRow : m_TextArray)
	{
		if (!Assert(this, textRow.size() == m_width, std::format("Tried to init a text array "
			"with width ({}) that does not match character arg ({})!",
			std::to_string(textRow.size()), std::to_string(m_width)))) return;
	}
}

TextArray::TextArray(const int& width, const int& height, const TextChar& duplicateBufferChar) :
	m_width(width), m_height(height), m_TextArray(CreateBufferOfChar(width, height, duplicateBufferChar))
{

}

TextArray::TextArray(const int& width, const int& height,
	const Color& color, const std::vector<std::vector<char>>& chars) :
	TextArray(width, height, CreateBufferOfChar(color, chars)) {}

TextArray::TextArray(const TextArray& other) :
	m_width(other.m_width), m_height(other.m_height),
	m_TextArray(other.m_TextArray)
{
	//Log(std::format("Invoking move constructor on: {} with other: {}", ToString(), other.ToString()));
}

TextArray::TextArray(TextArray&& other) noexcept :
	m_width(std::move(other.m_width)), m_height(std::move(other.m_height)),
	m_TextArray(std::move(other.m_TextArray))
{
	/*Log("Invoking copy constructor");*/
}

std::vector<std::vector<TextChar>> TextArray::CreateBufferOfChar(const int& width,
	const int& height, const TextChar& duplicateBufferChar) const
{
	std::vector<std::vector<TextChar>> chars = {};

	chars.reserve(height);
	for (int r = 0; r < height; r++)
	{
		chars.push_back({});
		chars[r].reserve(width);
		for (int c = 0; c < width; c++)
		{
			chars[r].push_back(duplicateBufferChar);
		}
	}
	return chars;
}

std::vector<std::vector<TextChar>> TextArray::CreateBufferOfChar(const Color& color,
	const std::vector<std::vector<char>>& chars) const
{
	std::vector<std::vector<TextChar>> result = {};

	result.reserve(chars.size());
	for (int r = 0; r < chars.size(); r++)
	{
		result.push_back({});
		result[r].reserve(chars[r].size());
		for (int c = 0; c < chars[r].size(); c++)
		{
			result[r].emplace_back(color, chars[r][c]);
		}
	}
	return result;
}

int TextArray::GetWidth() const
{
	return m_width;
}

int TextArray::GetHeight() const
{
	return m_height;
}

Utils::Point2DInt TextArray::GetSize() const
{
	return Utils::Point2DInt(GetWidth(), GetHeight());
}

bool TextArray::IsValidRow(const int& rowPos) const
{
	return 0 <= rowPos && rowPos < m_height;
}

bool TextArray::IsValidCol(const int& colPos) const
{
	return 0 <= colPos && colPos < m_width;
}

bool TextArray::IsValidPos(const Array2DPosition& rowColPos) const
{
	return IsValidRow(rowColPos.GetRow()) && IsValidCol(rowColPos.GetCol());
}

void TextArray::SetAt(const Array2DPosition& rowColPos, const TextChar& newBufferChar)
{
	if (!Assert(this, IsValidPos(rowColPos), std::format("Tried to set the char: {} "
		"at INVALID row col: {} of full buffer: {}",
		Utils::ToString(newBufferChar.m_Char), rowColPos.ToString(), ToString()))) return;

	m_TextArray[rowColPos.GetRow()][rowColPos.GetCol()].m_Char = newBufferChar.m_Char;
	m_TextArray[rowColPos.GetRow()][rowColPos.GetCol()].m_Color = newBufferChar.m_Color;
}

void TextArray::SetAt(const Array2DPosition& rowColPos, const char& newChar)
{
	if (!Assert(this, IsValidPos(rowColPos), std::format("Tried to set the char: {} "
		"at INVALID row col: {} of full buffer: {}",
		Utils::ToString(newChar), rowColPos.ToString(), ToString()))) return;

	m_TextArray[rowColPos.GetRow()][rowColPos.GetCol()].m_Char = newChar;
}

void TextArray::SetAt(const Array2DPosition& rowColPos, const Color& newColor)
{
	if (!Assert(this, IsValidPos(rowColPos), 
		std::format("Tried to set the color: {} at INVALID row col: {} of full buffer: {}",
		RaylibUtils::ToString(newColor), rowColPos.ToString(), ToString()))) return;

	m_TextArray[rowColPos.GetRow()][rowColPos.GetCol()].m_Color = newColor;
}

void TextArray::SetAt(const std::vector<Array2DPosition>& rowColPos, const TextChar& newBufferChar)
{
	for (const auto& pos : rowColPos)
	{
		SetAt(rowColPos, newBufferChar);
	}
}

void TextArray::SetAt(const std::vector<TextCharPosition>& updatedCharsAtPos)
{
	for (const auto& posChar : updatedCharsAtPos)
	{
		SetAt(posChar.m_RowColPos, posChar.m_Text);
	}
}

void TextArray::SetAt(const std::vector<ColorPosition>& updateColorsAtPos)
{
	for (const auto& colorPos : updateColorsAtPos)
	{
		SetAt(colorPos.m_RowColPos, colorPos.m_Color);
	}
}

bool TextArray::TrySetRegion(const Array2DPosition& rowColStartPos, const Utils::Point2DInt& size,
	const std::vector<std::vector<TextChar>>& chars)
{
	//Subtract one from width and col since start pos is inclusive
	Array2DPosition rowColEndPos = rowColStartPos + GetAsArray2DPos(size) + Array2DPosition(-1, -1);
	if (!Assert(this, IsValidPos(rowColEndPos), "Tried to set text buffer region but size is too big!"))
		return false;

	if (!Assert(this, chars.size() == size.m_Y, "Tried to set text buffer region but HEIGHT "
		"size does not match provided chars"))
		return false;

	Array2DPosition globalRowCol = {};
	for (int r = 0; r <= chars.size(); r++)
	{
		if (!Assert(this, chars[r].size() == size.m_X, "Tried to set text buffer region but WIDTH "
			"size does not match provided chars"))
			return false;

		for (int c = 0; c <= chars[r].size(); c++)
		{
			globalRowCol = { rowColStartPos.GetRow() + r, rowColStartPos.GetCol() + c };
			SetAt(globalRowCol, chars[r][c]);
		}
	}
	return true;
}

const TextChar* TextArray::GetAt(const Array2DPosition& rowColPos) const
{
	if (!Assert(this, IsValidPos(rowColPos), std::format("Tried to get INVALID pos at row col: {} of full buffer: {}",
		rowColPos.ToString(), ToString()))) return nullptr;

	/*Log(std::format("WHEN ACCESSING POS {} char: {} color is: {}",
		rowColPos.ToString(), Utils::ToString(m_TextArray[rowColPos.m_X][rowColPos.m_Y].m_Char),
		RaylibUtils::ToString(m_TextArray[rowColPos.m_X][rowColPos.m_Y].m_Color)));*/

	return &(m_TextArray[rowColPos.GetRow()][rowColPos.GetCol()]);
}

const TextChar& TextArray::GetAtUnsafe(const Array2DPosition& rowColPos) const
{
	return m_TextArray[rowColPos.GetRow()][rowColPos.GetCol()];
}

const std::vector<TextChar>& TextArray::GetAt(const int& rowPos) const
{
	if (!Assert(this, IsValidRow(rowPos), std::format("Tried to get INVALID row pos {} of full buffer: {}",
		std::to_string(rowPos), ToString()))) return {};

	return m_TextArray[rowPos];
}

std::string TextArray::GetStringAt(const int& rowColPos) const
{
	if (!Assert(this, IsValidRow(rowColPos), std::format("Tried to get INVALID row pos {} of full buffer: {}",
		std::to_string(rowColPos), ToString()))) return {};

	std::string rowStr = "";
	for (const auto& textChar : m_TextArray[rowColPos])
	{
		rowStr += textChar.m_Char;
	}
	return rowStr;
}

const TextArrayCollectionType& TextArray::GetFull() const
{
	return m_TextArray;
}

std::string TextArray::ToString(const std::vector<std::vector<TextChar>>& buffer,
	const bool convertAll)
{
	std::string fullStr = "";
	char currentChar = '0';
	for (int r = 0; r < buffer.size(); r++)
	{
		for (int c = 0; c < buffer[r].size(); c++)
		{
			currentChar = buffer[r][c].m_Char;
			fullStr += currentChar;

			if (convertAll)
			{
				fullStr += RaylibUtils::ToString(buffer[r][c].m_Color) + " ";
			}
			/*if (currentChar == EMPTY_CHAR_PLACEHOLDER) fullStr += " ";
			else fullStr += currentChar;*/
		}
		fullStr += "\n";
	}
	return fullStr;
}

std::string TextArray::ToString(bool convertChars) const
{
	return std::format("(W:{} H:{})",
		std::to_string(m_width), std::to_string(m_height)) + ToString(m_TextArray, convertChars);
}

TextArray& TextArray::operator=(const TextArray& other)
{
	//Log("USING TEXT BUFFER LVALUE = OPERATOR");
	if (&other == this) return *this;
	/*Log("COPY ASSIGNMNT");*/
	//Log(std::format("ASSIGNGIN BUFFER: {} to {}", other.ToString(), ToString()));
	//Log(std::format("ASSIGNGIN BUFFER: {} -> {}", other.ToString(), ToString()));

	//m_TextArray = other.m_TextArray;
	m_TextArray = other.m_TextArray;
	m_width = other.m_width;
	m_height = other.m_height;
	return *this;
}

TextArray& TextArray::operator=(TextArray&& other) noexcept
{
	//Log("USING TEXT BUFFER RVALUE = OPERATOR");
	if (this == &other)
		return *this;

	/*Log("MOVE ASSIGNMNT");*/
	m_TextArray = std::exchange(other.m_TextArray, {});
	m_width = std::exchange(other.m_width, 0);
	m_height = std::exchange(other.m_height, 0);

	return *this;
}
