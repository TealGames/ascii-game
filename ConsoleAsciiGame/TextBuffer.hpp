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

struct TextCharPosition
{
	Utils::Point2DInt m_RowColPos;
	TextChar m_Text;
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
	void SetAt(const Utils::Point2DInt& rowColPos, const char& newChar);
	void SetAt(const Utils::Point2DInt& rowColPos, const Color& newColor);

	void SetAt(const std::vector<Utils::Point2DInt>& rowColPos, const TextChar& newBufferChar);
	void SetAt(const std::vector<TextCharPosition>& updatedCharsAtPos);

	/// <summary>
	/// Will set the region from row col start pos with size [WIDTH, HEIGHT]
	/// </summary>
	/// <param name="rowColStartPos"></param>
	/// <param name="size"></param>
	/// <param name="chars"></param>
	/// <returns></returns>
	bool TrySetRegion(const Utils::Point2DInt& rowColStartPos, const Utils::Point2DInt& size, 
		const std::vector<std::vector<TextChar>>& chars);

	const TextChar* GetAt(const Utils::Point2DInt& rowColPos) const;
	const std::vector<TextChar>& GetAt(const int& rowPos) const;

	std::string ToString(bool convertChars=true) const;
};

