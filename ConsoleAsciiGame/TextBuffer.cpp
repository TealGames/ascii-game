#include "pch.hpp"
#include "raylib.h"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"

TextChar::TextChar() :
	m_Color{}, m_Char(EMPTY_CHAR_PLACEHOLDER) {}

TextChar::TextChar(const Color& color, const char& textChar = EMPTY_CHAR_PLACEHOLDER) :
	m_Color(color), m_Char(textChar) {}

std::string TextChar::ToString() const
{
	return std::format("[Color: {} C:{}]", 
		RaylibUtils::ToString(m_Color), Utils::ToString(m_Char));
}

TextCharPosition::TextCharPosition(const Utils::Point2DInt pos, const TextChar& textChar) 
	: m_RowColPos(pos), m_Text(textChar)
{

}

std::string TextCharPosition::ToString() const
{
	return std::format("[Pos:{}, Data:{}]", 
		m_RowColPos.ToString(), m_Text.ToString());
}

TextBuffer::TextBuffer() :
	TextBuffer(0, 0, std::vector<std::vector<TextChar>>{ {TextChar(Color(), EMPTY_CHAR_PLACEHOLDER)} }) {}

TextBuffer::TextBuffer(const int& width, const int& height, const std::vector<std::vector<TextChar>>& chars) :
	m_width(width), m_height(height), m_textBuffer(chars)
{
	if (m_height == 0) return;
	if (chars.size() != m_height)
	{
		const std::string err = std::format("Tried to init a text buffer with height ({}) that does not match character arg ({})!",
			std::to_string(chars.size()), std::to_string(m_height));
		Utils::Log(Utils::LogType::Error, err);
		return;
	}

	for (auto& textRow : m_textBuffer)
	{
		if (textRow.size() != m_width)
		{
			const std::string err = std::format("Tried to init a text buffer with width ({}) that does not match character arg ({})!",
				std::to_string(textRow.size()), std::to_string(m_width));
			Utils::Log(Utils::LogType::Error, err);
			return;
		}
	}
}

TextBuffer::TextBuffer(const int& width, const int& height, const TextChar& duplicateBufferChar) :
	m_width(width), m_height(height), m_textBuffer(CreateBufferOfChar(width, height, duplicateBufferChar))
{
	Utils::Log("DUPLICATE CHAR TEXT BUFFER");
}

TextBuffer::TextBuffer(const int& width, const int& height,
	const Color& color, const std::vector<std::vector<char>>& chars) :
	TextBuffer(width, height, CreateBufferOfChar(color, chars)) {}

TextBuffer::TextBuffer(const TextBuffer& other) :
	m_width(other.m_width), m_height(other.m_height),
	m_textBuffer(other.m_textBuffer) 
{
	//Utils::Log(std::format("Invoking move constructor on: {} with other: {}", ToString(), other.ToString()));
}

TextBuffer::TextBuffer(TextBuffer&& other) noexcept : 
	m_width(std::move(other.m_width)), m_height(std::move(other.m_height)),
	m_textBuffer(std::move(other.m_textBuffer))
{
	/*Utils::Log("Invoking copy constructor");*/
}

std::vector<std::vector<TextChar>> TextBuffer::CreateBufferOfChar(const int& width, 
	const int& height, const TextChar& duplicateBufferChar) const
{
	std::vector<std::vector<TextChar>> chars = {};
	
	chars.reserve(height);
	for (int r=0; r< height; r++)
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

std::vector<std::vector<TextChar>> TextBuffer::CreateBufferOfChar(const Color& color,
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
	for (const auto& row : result) Utils::Log(std::format("CREATED ROW: {}", Utils::ToStringIterable<std::vector<TextChar>, TextChar>(row)));
	return result;
}

int TextBuffer::GetWidth() const
{
	return m_width;
}

int TextBuffer::GetHeight() const
{
	return m_height;
}

Utils::Point2DInt TextBuffer::GetSize() const
{
	return Utils::Point2DInt(GetWidth(), GetHeight());
}

bool TextBuffer::IsValidRow(const int& rowPos) const
{
	return 0 <= rowPos && rowPos < m_height;
}

bool TextBuffer::IsValidCol(const int& colPos) const
{
	return 0 <= colPos && colPos < m_width;
}

bool TextBuffer::IsValidPos(const Utils::Point2DInt& rowColPos) const
{
	return IsValidRow(rowColPos.m_X) && IsValidCol(rowColPos.m_Y);
}

void TextBuffer::SetAt(const Utils::Point2DInt& rowColPos, const TextChar& newBufferChar)
{
	if (!Utils::Assert(IsValidPos(rowColPos), std::format("Tried to set the char: {} at INVALID row col: {} of full buffer: {}", 
		Utils::ToString(newBufferChar.m_Char), rowColPos.ToString(), ToString()))) return;

	m_textBuffer[rowColPos.m_X][rowColPos.m_Y].m_Char = newBufferChar.m_Char;
	m_textBuffer[rowColPos.m_X][rowColPos.m_Y].m_Color = newBufferChar.m_Color;
}

void TextBuffer::SetAt(const Utils::Point2DInt& rowColPos, const char& newChar)
{
	if (!Utils::Assert(IsValidPos(rowColPos), std::format("Tried to set the char: {} at INVALID row col: {} of full buffer: {}",
		Utils::ToString(newChar), rowColPos.ToString(), ToString()))) return;

	m_textBuffer[rowColPos.m_X][rowColPos.m_Y].m_Char = newChar;
}

void TextBuffer::SetAt(const Utils::Point2DInt& rowColPos, const Color& newColor)
{
	if (!Utils::Assert(IsValidPos(rowColPos), std::format("Tried to set the color: {} at INVALID row col: {} of full buffer: {}",
		RaylibUtils::ToString(newColor), rowColPos.ToString(), ToString()))) return;

	m_textBuffer[rowColPos.m_X][rowColPos.m_Y].m_Color = newColor;
}

void TextBuffer::SetAt(const std::vector<Utils::Point2DInt>& rowColPos, const TextChar& newBufferChar)
{
	for (const auto& pos : rowColPos)
	{
		SetAt(rowColPos, newBufferChar);
	}
}

void TextBuffer::SetAt(const std::vector<TextCharPosition>& updatedCharsAtPos)
{
	for (const auto& posChar : updatedCharsAtPos)
	{
		SetAt(posChar.m_RowColPos, posChar.m_Text);
	}
}

bool TextBuffer::TrySetRegion(const Utils::Point2DInt& rowColStartPos, const Utils::Point2DInt& size,
	const std::vector<std::vector<TextChar>>& chars)
{
	//Subtract one from width and col since start pos is inclusive
	Utils::Point2DInt rowColEndPos = rowColStartPos + size + Utils::Point2DInt(-1, -1);
	if (!Utils::Assert(IsValidPos(rowColEndPos), "Tried to set text buffer region but size is too big!"))
		return false;

	if (!Utils::Assert(chars.size()==size.m_Y, "Tried to set text buffer region but HEIGHT "
		"size does not match provided chars"))
		return false;

	Utils::Point2DInt globalRowCol = {};
	for (int r = 0; r <= chars.size(); r++)
	{
		if (!Utils::Assert(chars[r].size() == size.m_X, "Tried to set text buffer region but WIDTH "
			"size does not match provided chars"))
			return false;

		for (int c = 0; c <= chars[r].size(); c++)
		{
			globalRowCol = { rowColStartPos.m_X+r, rowColStartPos.m_Y+c };
			SetAt(globalRowCol, chars[r][c]);
		}
	}
}

const TextChar* TextBuffer::GetAt(const Utils::Point2DInt& rowColPos) const
{
	if (!Utils::Assert(IsValidPos(rowColPos), std::format("Tried to get INVALID pos at row col: {} of full buffer: {}",
		rowColPos.ToString(), ToString()))) return nullptr;

	/*Utils::Log(std::format("WHEN ACCESSING POS {} char: {} color is: {}", 
		rowColPos.ToString(), Utils::ToString(m_textBuffer[rowColPos.m_X][rowColPos.m_Y].m_Char), 
		RaylibUtils::ToString(m_textBuffer[rowColPos.m_X][rowColPos.m_Y].m_Color)));*/

	return &(m_textBuffer[rowColPos.m_X][rowColPos.m_Y]);
}

const TextChar& TextBuffer::GetAtUnsafe(const Utils::Point2DInt& rowColPos) const
{
	return m_textBuffer[rowColPos.m_X][rowColPos.m_Y];
}

const std::vector<TextChar>& TextBuffer::GetAt(const int& rowPos) const
{
	if (!Utils::Assert(IsValidRow(rowPos), std::format("Tried to get INVALID row pos {} of full buffer: {}",
		std::to_string(rowPos), ToString()))) return {};

	return m_textBuffer[rowPos];
}

std::string TextBuffer::ToString(const std::vector<std::vector<TextChar>>& buffer,
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

std::string TextBuffer::ToString(bool convertChars) const
{
	return std::format("(W:{} H:{})", 
		std::to_string(m_width), std::to_string(m_height)) + ToString(m_textBuffer, convertChars);
}

TextBuffer& TextBuffer::operator=(const TextBuffer& other)
{
	Utils::Log("USING TEXT BUFFER LVALUE = OPERATOR");
	if (&other == this) return *this;
	/*Utils::Log("COPY ASSIGNMNT");*/
	//Utils::Log(std::format("ASSIGNGIN BUFFER: {} to {}", other.ToString(), ToString()));
	//Utils::Log(std::format("ASSIGNGIN BUFFER: {} -> {}", other.ToString(), ToString()));

	//m_textBuffer = other.m_textBuffer;
	m_textBuffer = other.m_textBuffer;
	m_width = other.m_width;
	m_height = other.m_height;
	return *this;
}

TextBuffer& TextBuffer::operator=(TextBuffer&& other) noexcept
{
	Utils::Log("USING TEXT BUFFER RVALUE = OPERATOR");
	if (this == &other)
		return *this;

	/*Utils::Log("MOVE ASSIGNMNT");*/
	m_textBuffer = std::exchange(other.m_textBuffer, {});
	m_width = std::exchange(other.m_width, 0);
	m_height = std::exchange(other.m_height, 0);

	return *this;
}