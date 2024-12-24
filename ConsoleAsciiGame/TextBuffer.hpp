#pragma once
#include <vector>
#include <string>
#include "Point2DInt.hpp"
#include "raylib.h"

//The character used for empty spaces and will NOT be rendered
constexpr char EMPTY_CHAR_PLACEHOLDER = '_';

//TODO: optimization could be to group together similar colors
//into one batch that contains the positions, different chars and color
struct TextChar
{
	Color m_Color;
	char m_Char;

	TextChar();
	TextChar(const Color& color, const char& textChar);
	TextChar(const TextChar&) = default;

	bool operator==(const TextChar& other) const = default;
};

class TextBuffer
{
private:
	std::vector<std::vector<TextChar>> m_textBuffer;
public:
	const int m_WIDTH;
	const int m_HEIGHT;

private:
public:
	TextBuffer(const int& width, const int& height, const std::vector<std::vector<TextChar>>& chars);
	TextBuffer(const int& width, const int& height, const TextChar& duplicateBufferChar);

	bool IsValidRow(const int& rowPos) const;
	bool IsValidCol(const int& colPos) const;
	bool IsValidPos(const Utils::Point2DInt& rowColPos) const;

	void SetAt(const Utils::Point2DInt& rowColPos, const TextChar& newBufferChar);
	void SetAt(const std::vector<Utils::Point2DInt>& rowColPos, const TextChar& newBufferChar);
	void SetAt(const Utils::Point2DInt& rowColPos, const char& newChar);
	void SetAt(const Utils::Point2DInt& rowColPos, const Color& newColor);

	const TextChar* GetAt(const Utils::Point2DInt& rowColPos) const;
	const std::vector<TextChar>& GetAt(const int& rowPos) const;

	std::string ToString(bool convertChars=true) const;
};

