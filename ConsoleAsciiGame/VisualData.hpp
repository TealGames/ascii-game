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

/// <summary>
/// The type of character spacing used for text of visual
/// </summary>
enum class CharAreaType
{
	/// <summary>
	/// Will use a predefined width and height for any character
	/// </summary>
	Predefined,
	/// <summary>
	/// Will use the actual size taken up by the character
	/// </summary>
	Adaptive,
};

using RawTextBufferBlock = std::vector<std::vector<TextCharPosition>>;
class VisualData
{
public:
	static const float& DEFAULT_FONT_SIZE;
	static const Utils::Point2D& DEFAULT_CHAR_SPACING;
	static const Utils::Point2D& DEFAULT_PREDEFINED_CHAR_AREA;

	static const Utils::Point2D PIVOT_TOP_LEFT;
	static const Utils::Point2D PIVOT_TOP_RIGHT;
	static const Utils::Point2D PIVOT_BOTTOM_LEFT;
	static const Utils::Point2D PIVOT_BOTTOM_RIGHT;
	static const Utils::Point2D PIVOT_CENTER;
	static const Utils::Point2D PIVOT_BOTTOM_CENTER;
	static const Utils::Point2D PIVOT_TOP_CENTER;

	static const Utils::Point2D DEFAULT_PIVOT;

private:
	/// <summary>
	/// The spacing between characters in the visual in [WIDTH, HEIGHT]
	/// </summary>
	Utils::Point2D m_charSpacing;

	CharAreaType m_charAreaType;
	Utils::Point2D m_predefinedCharArea;

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

	WorldPosition GetTopLeftPos(const WorldPosition& pivotWorldPos, const Utils::Point2D& totalSize) const;
	void AddTextPositionsToBufferPredefined(const WorldPosition& transformPos, TextBufferMixed& buffer) const;
	void AddTextPositionsToBufferAdaptive(const WorldPosition& transformPos, TextBufferMixed& buffer) const;

private:
	VisualData(const RawTextBufferBlock& rawBuffer, const Font& font,
		const float& fontSize, const Utils::Point2D& charSpacing,
		const Utils::Point2D& relativePivotPos, const CharAreaType& charAreaType,
		const Utils::Point2D& predefinedCharArea);

public:
	VisualData();
	/// <summary>
	/// This constructor is used for the adaptive char area for the text
	/// </summary>
	/// <param name="rawBuffer"></param>
	/// <param name="font"></param>
	/// <param name="fontSize"></param>
	/// <param name="charSpacing"></param>
	/// <param name="relativePivotPos"></param>
	VisualData(const RawTextBufferBlock& rawBuffer, const Font& font, 
		const float& fontSize, const Utils::Point2D& charSpacing, 
		const Utils::Point2D& relativePivotPos);

	/// <summary>
	/// This constructor applies to predefined char area for text
	/// </summary>
	/// <param name="rawBuffer"></param>
	/// <param name="font"></param>
	/// <param name="fontSize"></param>
	/// <param name="charSpacing"></param>
	/// <param name="predefinedCharArea"></param>
	/// <param name="relativePivotPos"></param>
	VisualData(const RawTextBufferBlock& rawBuffer, const Font& font,
		const float& fontSize, const Utils::Point2D& charSpacing,
		const Utils::Point2D& predefinedCharArea,
		const Utils::Point2D& relativePivotPos);
	
	std::optional<TextArray> CreateSquaredBuffer(const RawTextBufferBlock& rawBuffer) const;
	std::string ToStringRawBuffer(const RawTextBufferBlock& block);

	Utils::Point2D GetWorldSize() const;
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
