#pragma once
#include <vector>
#include "Core/Visual/TextArray.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Utils/Data/Vec2Type.hpp"
#include "Utils/Data/NormalizedPosition.hpp" 
#include "Utils/Data/WorldPosition.hpp"

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
	const FontAsset* m_FontAsset = nullptr;
	float m_FontSize = 0;
	Vec2 m_CharSpacing = {};
	CharAreaType m_CharAreaType = CharAreaType::Adaptive;
	Vec2 m_PredefinedCharArea = {};
	NormalizedPosition m_RelativePivotPos = {};

	VisualDataPreset(const FontAsset& font, const float& fontSize, const Vec2& charSpacing,
		const CharAreaType& charAreaType, const Vec2& predefinedCharArea, 
		const NormalizedPosition& relativePivotPos);
};

//using RawTextBufferBlock = std::vector<std::vector<TextCharArrayPosition>>;
//std::string ToString(const RawTextBufferBlock& rawBuffer);
class VisualData
{
public:
	static const Vec2& DEFAULT_FONT_SIZE;

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
	//RawTextBufferBlock m_rawTextBlock;

	///// <summary>
	///// The spacing between characters in the visual in [WIDTH, HEIGHT]
	///// </summary>
	//Vec2 m_charSpacing;

	//CharAreaType m_charAreaType;
	//Vec2 m_predefinedCharArea;

	/// <summary>
	/// The position relative to the visual that corresponds to the transform position.
	/// Note: (0,0) refers to bottom left, (1,1) refers to top right
	/// </summary>
	NormalizedPosition m_pivotRelative;

	//FontData m_fontData;

	/// <summary>
	/// This holds all of the characters and their positions from the pivot 
	/// NOTE: all positions all based on the left side (image each char is on a rect, the top left pos is
	/// the coorindate used for the buffer)
	/// </summary>
	FragmentedTextBuffer m_buffer;
	Vec2 m_worldSize;
public:

private:
	//bool HasValidFont(const bool assertMessage= true, const std::string& extraMessage="") const;

	//WorldPosition GetTopLeftPos(const WorldPosition& pivotWorldPos, const Vec2& totalSize) const;
	/*void AddTextPositionsToBufferPredefined(const WorldPosition& transformPos, TextBufferMixed& buffer) const;
	void AddTextPositionsToBufferAdaptive(const WorldPosition& transformPos, TextBufferMixed& buffer) const;*/

private:
	void CreateBuffer(const std::vector<std::vector<TextBufferChar>>& rawBuffer, const Vec2& charSpacing);

	void AddToCreatedBuffer(const size_t& r, const size_t& c, const size_t currRowElementCount, const TextChar& textChar, 
		const WorldFontProperties& fontData, const Vec2& charSpacing, Vec2& pivotDiff, 
		NormalizedPosition& currPosNormalized, const Vec2& fullSize, float* currentRowMaxHeight, const Vec2* predefinedCharArea);

public:
	VisualData();

	/// <summary>
	/// This overload uses a vector of data that contains the char position, color, text, font, etc.
	/// Most useful for having lots of varied data with lots of holes/uneven spacing and compelx chapes
	/// </summary>
	/// <param name="rawBuffer"></param>
	/// <param name="relativePivotPos"></param>
	VisualData(const FragmentedTextBuffer& rawBuffer, const NormalizedPosition& relativePivotPos);

	/// <summary>
	/// This overload uses a 2d array of data with char, color and font
	/// Most useful for box/non-complex shapes with each character having their own color/font settings
	/// </summary>
	/// <param name="rawBuffer"></param>
	/// <param name="charSpacing"></param>
	/// <param name="relativePivotPos"></param>
	VisualData(const std::vector<std::vector<TextBufferChar>>& rawBuffer, const Vec2& charSpacing,
		const NormalizedPosition& relativePivotPos);

	/// <summary>
	/// This overload uses a 2d array of char and color and a global font to use for all elementss
	/// Most useful for box-shapes structures each with similar font but different color/character
	/// </summary>
	/// <param name="rawBuffer"></param>
	/// <param name="charSpacing"></param>
	/// <param name="fontSettings"></param>
	/// <param name="relativePivotPos"></param>
	VisualData(const std::vector<std::vector<TextChar>>& rawBuffer, const Vec2& charSpacing,
		const WorldFontProperties& fontSettings, const NormalizedPosition& relativePivotPos);

	//TODO: add overload with same char for every location and one for same color in every 2d element

	/// <summary>
	/// This overload uses a 2d array of char and color and a global font and global char area
	/// Most useful for box shapes with same font and where each element occupies same amount of space regardless of font
	/// </summary>
	VisualData(const std::vector<std::vector<TextChar>>& rawBuffer, const Vec2& charArea, const Vec2& charSpacing,
		const WorldFontProperties& fontSettings, const NormalizedPosition& relativePivotPos);

	/*/// <summary>
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

	VisualData(const RawTextBufferBlock& rawBuffer, const VisualDataPreset& preset);*/
	
	//FragmentedTextBuffer CreateRectangularBuffer(const std::vector<std::vector<TextBufferChar>>& rawBuffer) const;

	//const RawTextBufferBlock& GetRawBuffer() const;

	//Vec2Int GetBufferSize() const;
	bool IsEmpty() const;
	Vec2 GetWorldSize() const;
	/// <summary>
	/// Pivot position is relative to origin of bottom left [0,0] and top right [1, 1]
	/// </summary>
	/// <param name="pivotPosition"></param>
	/// <param name="transformPos"></param>
	/// <returns></returns>
	//void AddTextPositionsToBuffer(const WorldPosition& globalTransformPos, FragmentedTextBuffer& buffer) const;
	const FragmentedTextBuffer& GetBuffer() const;
	/*const Vec2& GetCharSpacing() const;
	const Font& GetFont() const;
	float GetFontSize() const;
	const FontProperties& GetFontData() const;*/

	Vec2 GetPivotRelative() const;
	/// <summary>
	/// Gets the pivot position in the world.
	/// Note: since visual is 2d, the whole visual will inherit the center pos z coord
	/// </summary>
	/// <param name="centerScreenPos"></param>
	/// <returns></returns>
	WorldPosition3D GetPivotWorldPos(const WorldPosition3D& centerScreenPos) const;

	//void AddVisualLocationToBuffer(const size_t& r, const size_t& c, FragmentedTextBuffer& buffer, const ) const;

	/*void SetPredefinedCharArea(const Vec2& area);
	void SetAdpativeCharArea();
	bool HasPredefinedCharArea() const;
	const Vec2& GetPredefinedCharArea() const;*/

	std::string ToString() const;
};

//TODO: should sprites have emptyu char for locations not covered or should they store the position?
using VisualDataPositions = std::vector<TextCharArrayPosition>;
