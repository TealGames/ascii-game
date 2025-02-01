#pragma once
#include <vector>
#include "TextArray.hpp"
#include "FragmentedTextArray.hpp"
#include "raylib.h"
#include <cstdint>
#include <optional>
#include "Globals.hpp"
#include "TextBuffer.hpp"

//TODO: the data from get world size of visual data should be abstracted
//so the entity renderer does not have to guess where top left pos is
struct VisualSizeInfo
{
	Utils::Point2D m_TextSize;
	Utils::Point2D m_TotalSize;
};

using RawTextBufferBlock = std::vector<std::vector<TextCharPosition>>;
class VisualData
{
public:
	const float& DEFAULT_FONT_SIZE = GLOBAL_FONT_SIZE;
	const Utils::Point2D& DEFAULT_CHAR_SPACING = GLOBAL_CHAR_SPACING;

	static const Utils::Point2D PIVOT_TOP_LEFT;
	static const Utils::Point2D PIVOT_TOP_RIGHT;
	static const Utils::Point2D PIVOT_BOTTOM_LEFT;
	static const Utils::Point2D PIVOT_BOTTOM_RIGHT;
	static const Utils::Point2D PIVOT_CENTER;
	static const Utils::Point2D PIVOT_BOTTOM_CENTER;
	static const Utils::Point2D PIVOT_TOP_CENTER;

private:
	/// <summary>
	/// The spacing between characters in the visual in [WIDTH, HEIGHT]
	/// </summary>
	Utils::Point2D m_charSpacing;

	/// <summary>
	/// The position relative to the visual that corresponds to the transform position.
	/// Note: (0,0) refers to bottom left, (1,1) refers to top right
	/// </summary>
	Utils::Point2D m_pivotRelative;

	const Font* m_font;
	float m_fontSize;

public:
	TextArray m_Text;

private:
	bool HasValidFont() const;
	bool ValidatePivot(Utils::Point2D& pivot) const;

public:
	VisualData();
	VisualData(const RawTextBufferBlock& rawBuffer, const Font& font, 
		const std::uint8_t& fontSize, const Utils::Point2D& charSpacing, const Utils::Point2D& relativePivotPos= PIVOT_TOP_LEFT);
	
	std::optional<TextArray> CreateSquaredBuffer(const RawTextBufferBlock& rawBuffer) const;
	std::string ToStringRawBuffer(const RawTextBufferBlock& block);

	Utils::Point2DInt GetWorldSize() const;
	/// <summary>
	/// Pivot position is relative to origin of bottom left [0,0] and top right [1, 1]
	/// </summary>
	/// <param name="pivotPosition"></param>
	/// <param name="transformPos"></param>
	/// <returns></returns>
	void AddTextPositionsToBuffer(const WorldPosition& transformPos, TextBufferMixed& buffer) const;

	const Utils::Point2D& GetCharSpacing() const;
	const Font& GetFont() const;
	const float GetFontSize() const;

	const Utils::Point2D& GetPivot() const;
};

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
