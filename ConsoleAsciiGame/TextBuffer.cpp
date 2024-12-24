#include "raylib.h"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"

TextChar::TextChar() :
	m_Color{}, m_Char(EMPTY_CHAR_PLACEHOLDER) {}

TextChar::TextChar(const Color& color, const char& textChar = EMPTY_CHAR_PLACEHOLDER) :
	m_Color(color), m_Char(textChar) {}

TextBuffer::TextBuffer(const int& width, const int& height, const std::vector<std::vector<TextChar>>& chars) :
	m_WIDTH(width), m_HEIGHT(height), m_textBuffer({})
{
	if (chars.size() > m_HEIGHT)
	{
		const std::string err = std::format("Tried to init a text buffer with height ({}) greater than size specified ({})!",
			std::to_string(chars.size()), std::to_string(m_HEIGHT));
		Utils::Log(Utils::LogType::Error, err);
		return;
	}

	m_textBuffer.reserve(m_HEIGHT);
	for (auto& textRow : m_textBuffer)
	{
		if (chars.size() > m_WIDTH)
		{
			const std::string err = std::format("Tried to init a text buffer with width ({}) greater than size specified ({})!",
				std::to_string(chars.size()), std::to_string(m_WIDTH));
			Utils::Log(Utils::LogType::Error, err);
			return;
		}
		textRow.reserve(m_WIDTH);
	}
	m_textBuffer = chars;
}

TextBuffer::TextBuffer(const int& width, const int& height, const TextChar& duplicateBufferChar) :
	m_WIDTH(width), m_HEIGHT(height), m_textBuffer({})
{
	m_textBuffer.reserve(m_HEIGHT);
	for (int r=0; r<m_HEIGHT; r++)
	{
		m_textBuffer.reserve(m_WIDTH);
		m_textBuffer.push_back({});
		for (int c = 0; c < m_WIDTH; c++)
		{
			m_textBuffer[r].push_back(duplicateBufferChar);
		}
	}
}

bool TextBuffer::IsValidRow(const int& rowPos) const
{
	return 0 <= rowPos && rowPos < m_HEIGHT;
}

bool TextBuffer::IsValidCol(const int& colPos) const
{
	return 0 <= colPos && colPos < m_WIDTH;
}

bool TextBuffer::IsValidPos(const Utils::Point2DInt& rowColPos) const
{
	return IsValidRow(rowColPos.m_X) && IsValidCol(rowColPos.m_Y);
}

void TextBuffer::SetAt(const Utils::Point2DInt& rowColPos, const TextChar& newBufferChar)
{
	if (!IsValidPos(rowColPos))
	{
		return;
	}
	m_textBuffer[rowColPos.m_X][rowColPos.m_Y].m_Char = newBufferChar.m_Char;
	m_textBuffer[rowColPos.m_X][rowColPos.m_Y].m_Color = newBufferChar.m_Color;
}

void TextBuffer::SetAt(const std::vector<Utils::Point2DInt>& rowColPos, const TextChar& newBufferChar)
{
	for (const auto& pos : rowColPos)
	{
		SetAt(rowColPos, newBufferChar);
	}
}

void TextBuffer::SetAt(const Utils::Point2DInt& rowColPos, const char& newChar)
{
	if (!IsValidPos(rowColPos))
	{
		return;
	}
	m_textBuffer[rowColPos.m_X][rowColPos.m_Y].m_Char = newChar;
}

void TextBuffer::SetAt(const Utils::Point2DInt& rowColPos, const Color& newColor)
{
	if (!IsValidPos(rowColPos))
	{
		return;
	}
	m_textBuffer[rowColPos.m_X][rowColPos.m_Y].m_Color = newColor;
}

const TextChar* TextBuffer::GetAt(const Utils::Point2DInt& rowColPos) const
{
	if (!IsValidPos(rowColPos))
	{
		return nullptr;
	}
	return &(m_textBuffer[rowColPos.m_X][rowColPos.m_Y]);
}

const std::vector<TextChar>& TextBuffer::GetAt(const int& rowPos) const
{
	if (IsValidRow(rowPos))
	{
		return {};
	}
	return m_textBuffer[rowPos];
}

std::string TextBuffer::ToString(bool convertChars) const
{
	std::string fullStr = "";
	char currentChar = '0';
	for (int r = 0; r < m_HEIGHT; r++)
	{
		for (int c = 0; c < m_WIDTH; c++)
		{
			if (convertChars)
			{
				currentChar = GetAt({ r, c })->m_Char;
				fullStr += currentChar;
			}
			else
			{
				fullStr += RaylibUtils::ToString(GetAt({ r, c })->m_Color) +" ";
			}
			/*if (currentChar == EMPTY_CHAR_PLACEHOLDER) fullStr += " ";
			else fullStr += currentChar;*/
		}
		fullStr += "\n";
	}
	return fullStr;
}