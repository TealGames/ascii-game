#pragma once
#include <vector>
#include <string>
//#include <array>
#include "Array2DPosition.hpp"
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

	bool operator==(const TextChar& other) const;
	std::string ToString() const;
};

struct TextCharPosition
{
	Array2DPosition m_RowColPos;
	TextChar m_Text;

	TextCharPosition();
	TextCharPosition(const Array2DPosition& pos, const TextChar& textChar);
	TextCharPosition(const TextCharPosition&) = default;

	bool operator==(const TextCharPosition& other) const;
	TextCharPosition& operator=(const TextCharPosition& other);
	std::string ToString() const;
};

struct ColorPosition
{
	Array2DPosition m_RowColPos;
	Color m_Color;

	bool operator==(const ColorPosition& other) const = default;
	std::string ToString() const;

	ColorPosition(const Array2DPosition& pos, const Color& color);
};

using TextArrayCollectionType = std::vector<std::vector<TextChar>>;
class TextArray
{
private:
	//TODO: since we have set width and height it should probably be 2d array to avoid
	//unnecessarat heap allocations
	TextArrayCollectionType m_TextArray;
	int m_width;
	int m_height;

public:

private:
	std::vector<std::vector<TextChar>> CreateBufferOfChar(const int& width,
		const int& height, const TextChar& duplicateBufferChar) const;

	std::vector<std::vector<TextChar>> CreateBufferOfChar(const Color& color,
		const std::vector<std::vector<char>>& chars) const;

public:
	TextArray();
	TextArray(const int& width, const int& height, const std::vector<std::vector<TextChar>>& chars);
	TextArray(const int& width, const int& height, const TextChar& duplicateBufferChar);
	TextArray(const int& width, const int& height, const Color& color, const std::vector<std::vector<char>>& chars);
	TextArray(const TextArray& other);
	TextArray(TextArray&& other) noexcept;

	int GetWidth() const;
	int GetHeight() const;
	Vec2Int GetSize() const;

	bool IsValidRow(const int& rowPos) const;
	bool IsValidCol(const int& colPos) const;
	bool IsValidPos(const Array2DPosition& rowColPos) const;

	void SetAt(const Array2DPosition& rowColPos, const TextChar& newBufferChar);
	void SetAt(const Array2DPosition& rowColPos, const char& newChar);
	void SetAt(const Array2DPosition& rowColPos, const Color& newColor);

	void SetAt(const std::vector<Array2DPosition>& rowColPos, const TextChar& newBufferChar);
	void SetAt(const std::vector<TextCharPosition>& updatedCharsAtPos);
	void SetAt(const std::vector<ColorPosition>& updateColorsAtPos);

	/// <summary>
	/// Will set the region from row col start pos with size [WIDTH, HEIGHT]
	/// </summary>
	/// <param name="rowColStartPos"></param>
	/// <param name="size"></param>
	/// <param name="chars"></param>
	/// <returns></returns>
	bool TrySetRegion(const Array2DPosition& rowColStartPos, const Vec2Int& size,
		const std::vector<std::vector<TextChar>>& chars);

	const TextChar* GetAt(const Array2DPosition& rowColPos) const;
	const TextChar& GetAtUnsafe(const Array2DPosition& rowColPos) const;
	const std::vector<TextChar>& GetAt(const int& rowPos) const;
	std::string GetStringAt(const int& rowColPos) const;

	const TextArrayCollectionType& GetFull() const;

	static std::string ToString(const std::vector<std::vector<TextChar>>& buffer,
		const bool convertChars = true);
	std::string ToString(const bool convertChars = true) const;

	TextArray& operator=(const TextArray& other);
	TextArray& operator=(TextArray&& other) noexcept;
};

