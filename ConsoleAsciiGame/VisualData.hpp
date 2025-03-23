#pragma once
#include <vector>
#include "TextArray.hpp"
#include "FragmentedTextArray.hpp"
#include "raylib.h"
#include <cstdint>
#include <optional>
#include "Globals.hpp"
#include "TextBuffer.hpp"
#include "Vec2.hpp"
#include "NormalizedPosition.hpp"


////TODO: the data from get world size of visual data should be abstracted
////so the entity renderer does not have to guess where top left pos is
//struct VisualSizeInfo
//{
//	Vec2 m_TextSize;
//	Vec2 m_TotalSize;
//};

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

struct VisualDataPreset
{
	Font m_Font = {};
	float m_FontSize = 0;
	Vec2 m_CharSpacing = {};
	CharAreaType m_CharAreaType = CharAreaType::Adaptive;
	Vec2 m_PredefinedCharArea = {};
	NormalizedPosition m_RelativePivotPos = {};

	VisualDataPreset(const Font& font, const float& fontSize, const Vec2& charSpacing,
		const CharAreaType& charAreaType, const Vec2& predefinedCharArea, 
		const NormalizedPosition& relativePivotPos);
};

using RawTextBufferBlock = std::vector<std::vector<TextCharPosition>>;
class VisualData
{
public:
	static const float& DEFAULT_FONT_SIZE;
	static const Vec2& DEFAULT_CHAR_SPACING;
	static const Vec2& DEFAULT_PREDEFINED_CHAR_AREA;

	static const Vec2 PIVOT_TOP_LEFT;
	static const Vec2 PIVOT_TOP_RIGHT;
	static const Vec2 PIVOT_BOTTOM_LEFT;
	static const Vec2 PIVOT_BOTTOM_RIGHT;
	static const Vec2 PIVOT_CENTER;
	static const Vec2 PIVOT_BOTTOM_CENTER;
	static const Vec2 PIVOT_TOP_CENTER;

	static const Vec2 DEFAULT_PIVOT;

private:
	/// <summary>
	/// The raw text block that was then transformed into the rectangular text array 
	/// (by filling in skipped positions with empty spaces)
	/// </summary>
	RawTextBufferBlock m_rawTextBlock;

	/// <summary>
	/// The spacing between characters in the visual in [WIDTH, HEIGHT]
	/// </summary>
	Vec2 m_charSpacing;

	CharAreaType m_charAreaType;
	Vec2 m_predefinedCharArea;

	/// <summary>
	/// The position relative to the visual that corresponds to the transform position.
	/// Note: (0,0) refers to bottom left, (1,1) refers to top right
	/// </summary>
	NormalizedPosition m_pivotRelative;

	Font m_font;
	float m_fontSize;

public:
	TextArray m_Text;

private:
	bool HasValidFont(const bool assertMessage= true, const std::string& extraMessage="") const;

	WorldPosition GetTopLeftPos(const WorldPosition& pivotWorldPos, const Vec2& totalSize) const;
	void AddTextPositionsToBufferPredefined(const WorldPosition& transformPos, TextBufferMixed& buffer) const;
	void AddTextPositionsToBufferAdaptive(const WorldPosition& transformPos, TextBufferMixed& buffer) const;

private:
	VisualData(const RawTextBufferBlock& rawBuffer, const Font& font,
		const float& fontSize, const Vec2& charSpacing,
		const NormalizedPosition& relativePivotPos, const CharAreaType& charAreaType,
		const Vec2& predefinedCharArea);

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
		const float& fontSize, const Vec2& charSpacing, 
		const NormalizedPosition& relativePivotPos);

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
		const float& fontSize, const Vec2& charSpacing,
		const Vec2& predefinedCharArea,
		const NormalizedPosition& relativePivotPos);

	VisualData(const RawTextBufferBlock& rawBuffer, const VisualDataPreset& preset);
	
	std::optional<TextArray> CreateSquaredBuffer(const RawTextBufferBlock& rawBuffer) const;
	std::string ToStringRawBuffer(const RawTextBufferBlock& block);

	const RawTextBufferBlock& GetRawBuffer() const;

	Vec2 GetWorldSize() const;
	/// <summary>
	/// Pivot position is relative to origin of bottom left [0,0] and top right [1, 1]
	/// </summary>
	/// <param name="pivotPosition"></param>
	/// <param name="transformPos"></param>
	/// <returns></returns>
	void AddTextPositionsToBuffer(const WorldPosition& transformPos, TextBufferMixed& buffer) const;

	const Vec2& GetCharSpacing() const;
	const Font& GetFont() const;
	const float GetFontSize() const;

	const Vec2& GetPivot() const;

	void SetPredefinedCharArea(const Vec2& area);
	void SetAdpativeCharArea();
	bool HasPredefinedCharArea() const;
	const Vec2& GetPredefinedCharArea() const;

	std::string ToString() const;
};

//TODO: should sprites have emptyu char for locations not covered or should they store the position?
using VisualDataPositions = std::vector<TextCharPosition>;
