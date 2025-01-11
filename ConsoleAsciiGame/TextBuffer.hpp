#pragma once
#include <vector>
#include <string>
#include <array>
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
	std::string ToString() const;
};

struct TextCharPosition
{
	Utils::Point2DInt m_RowColPos;
	TextChar m_Text;

	bool operator==(const TextCharPosition& other) const = default;
	std::string ToString() const;

	TextCharPosition(const Utils::Point2DInt pos, const TextChar& textChar);
};

struct ColorPosition
{
	Utils::Point2DInt m_RowColPos;
	Color m_Color;
	
	bool operator==(const ColorPosition& other) const = default;
	std::string ToString() const;

	ColorPosition(const Utils::Point2DInt pos, const Color& color);
};

class TextBuffer
{
private:
	//TODO: since we have set width and height it should probably be 2d array to avoid
	//unnecessarat heap allocations
	std::vector<std::vector<TextChar>> m_textBuffer;
	int m_width;
	int m_height;

public:

private:
	std::vector<std::vector<TextChar>> CreateBufferOfChar(const int& width, 
		const int& height, const TextChar& duplicateBufferChar) const;

	std::vector<std::vector<TextChar>> CreateBufferOfChar(const Color& color, 
		const std::vector<std::vector<char>>& chars) const;

public:
	TextBuffer();
	TextBuffer(const int& width, const int& height, const std::vector<std::vector<TextChar>>& chars);
	TextBuffer(const int& width, const int& height, const TextChar& duplicateBufferChar);
	TextBuffer(const int& width, const int& height, const Color& color, const std::vector<std::vector<char>>& chars);
	TextBuffer(const TextBuffer& other);
	TextBuffer(TextBuffer&& other) noexcept;

	int GetWidth() const;
	int GetHeight() const;
	Utils::Point2DInt GetSize() const;

	bool IsValidRow(const int& rowPos) const;
	bool IsValidCol(const int& colPos) const;
	bool IsValidPos(const Utils::Point2DInt& rowColPos) const;

	void SetAt(const Utils::Point2DInt& rowColPos, const TextChar& newBufferChar);
	void SetAt(const Utils::Point2DInt& rowColPos, const char& newChar);
	void SetAt(const Utils::Point2DInt& rowColPos, const Color& newColor);

	void SetAt(const std::vector<Utils::Point2DInt>& rowColPos, const TextChar& newBufferChar);
	void SetAt(const std::vector<TextCharPosition>& updatedCharsAtPos);
	void SetAt(const std::vector<ColorPosition>& updateColorsAtPos);

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
	const TextChar& GetAtUnsafe(const Utils::Point2DInt& rowColPos) const;
	const std::vector<TextChar>& GetAt(const int& rowPos) const;

	static std::string ToString(const std::vector<std::vector<TextChar>>& buffer, 
		const bool convertChars = true);
	std::string ToString(const bool convertChars=true) const;

	TextBuffer& operator=(const TextBuffer& other);
	TextBuffer& operator=(TextBuffer&& other) noexcept;
};

