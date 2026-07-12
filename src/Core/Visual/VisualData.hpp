#pragma once
#include <vector>
#include "Core/Visual/TextArray.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Core/Primitives/Vector.hpp"
#include "Core/Primitives/NormalizedVec2.hpp" 
#include "Core/Primitives/WorldPosition.hpp"

namespace Engine::Rendering
{
	inline const NormalizedVec2 SPRITE_BOTTOM_LEFT = NormalizedVec2(NormalizedValue::MIN, NormalizedValue::MIN);
	inline const NormalizedVec2 SPRITE_TOP_LEFT = NormalizedVec2(NormalizedValue::MIN, NormalizedValue::MAX);
	inline const NormalizedVec2 SPRITE_TOP_RIGHT = NormalizedVec2(NormalizedValue::MAX, NormalizedValue::MAX);
	inline const NormalizedVec2 SPRITE_BOTTOM_RIGHT = NormalizedVec2(NormalizedValue::MAX, NormalizedValue::MIN);
	inline const NormalizedVec2 SPRITE_CENTER = NormalizedVec2(NormalizedValue::HALF, NormalizedValue::HALF);

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
		NormalizedVec2 m_RelativePivotPos = {};

		VisualDataPreset(const FontAsset& font, const float& fontSize, const Vec2& charSpacing,
			const CharAreaType& charAreaType, const Vec2& predefinedCharArea,
			const NormalizedVec2& relativePivotPos);
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
		NormalizedVec2 m_pivotRelative;

		//FontData m_fontData;

		/// <summary>
		/// This holds all of the characters and their positions from the pivot 
		/// NOTE: all positions all based on the left side (image each char is on a rect, the top left pos is
		/// the coorindate used for the buffer)
		/// </summary>
		FragmentedTextBuffer2D m_buffer;
		Vec2 m_worldSize;
	public:
		
	private:
		void CreateBuffer(const std::vector<std::vector<TextBufferChar>>& rawBuffer, const Vec2& charSpacing);

		void AddToCreatedBuffer(const size_t& r, const size_t& c, const size_t currRowElementCount, const TextChar& textChar,
			const WorldFontProperties& fontData, const Vec2& charSpacing, Vec2& pivotDiff,
			NormalizedVec2& currPosNormalized, const Vec2& fullSize, float* currentRowMaxHeight, const Vec2* predefinedCharArea);

	public:
		VisualData();

		/// <summary>
		/// This overload uses a vector of data that contains the char position, color, text, font, etc.
		/// Most useful for having lots of varied data with lots of holes/uneven spacing and compelx chapes
		/// </summary>
		/// <param name="rawBuffer"></param>
		/// <param name="relativePivotPos"></param>
		VisualData(const FragmentedTextBuffer2D& rawBuffer, const NormalizedVec2& relativePivotPos);

		/// <summary>
		/// This overload uses a 2d array of data with char, color and font
		/// Most useful for box/non-complex shapes with each character having their own color/font settings
		/// </summary>
		/// <param name="rawBuffer"></param>
		/// <param name="charSpacing"></param>
		/// <param name="relativePivotPos"></param>
		VisualData(const std::vector<std::vector<TextBufferChar>>& rawBuffer, const Vec2& charSpacing,
			const NormalizedVec2& relativePivotPos);

		/// <summary>
		/// This overload uses a 2d array of char and color and a global font to use for all elementss
		/// Most useful for box-shapes structures each with similar font but different color/character
		/// </summary>
		/// <param name="rawBuffer"></param>
		/// <param name="charSpacing"></param>
		/// <param name="fontSettings"></param>
		/// <param name="relativePivotPos"></param>
		VisualData(const std::vector<std::vector<TextChar>>& rawBuffer, const Vec2& charSpacing,
			const WorldFontProperties& fontSettings, const NormalizedVec2& relativePivotPos);

		//TODO: add overload with same char for every location and one for same color in every 2d element

		/// <summary>
		/// This overload uses a 2d array of char and color and a global font and global char area
		/// Most useful for box shapes with same font and where each element occupies same amount of space regardless of font
		/// </summary>
		VisualData(const std::vector<std::vector<TextChar>>& rawBuffer, const Vec2& charArea, const Vec2& charSpacing,
			const WorldFontProperties& fontSettings, const NormalizedVec2& relativePivotPos);

		bool IsEmpty() const;
		Vec2 GetWorldSize() const;
		/// <summary>
		/// Pivot position is relative to origin of bottom left [0,0] and top right [1, 1]
		/// </summary>
		/// <param name="pivotPosition"></param>
		/// <param name="transformPos"></param>
		/// <returns></returns>
		//void AddTextPositionsToBuffer(const WorldPosition& globalTransformPos, FragmentedTextBuffer& buffer) const;
		const FragmentedTextBuffer2D& GetBuffer() const;

		Vec2 GetPivotRelative() const;
		/// <summary>
		/// Gets the pivot position in the world.
		/// Note: since visual is 2d, the whole visual will inherit the center pos z coord
		/// </summary>
		/// <param name="centerScreenPos"></param>
		/// <returns></returns>
		WorldPosition3D GetPivotWorldPos(const WorldPosition3D& centerScreenPos) const;

		std::string ToString() const;
	};

	//TODO: should sprites have emptyu char for locations not covered or should they store the position?
	using VisualDataPositions = std::vector<TextCharArrayPosition>;
}

