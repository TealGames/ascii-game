#pragma once
#include "TextArray.hpp"
#include "raylib.h"
#include "WorldPosition.hpp"
#include "ScreenPosition.hpp"
#include "Point2D.hpp"
#include <cstdint>

//using TextBuffer = TextArray;
struct FontData
{
	float m_FontSize;
	const Font* m_Font;

	FontData(const float& fontSize, const Font& font);
};

//TODO: perhaps the camera or something should group together into a new structure with the same font and font size
//so that we do not need to repeat the same data for mutliple entries
struct TextBufferPosition
{
	Utils::Point2D m_Pos;
	FontData m_FontData;
	TextChar m_Text;

	bool operator==(const TextBufferPosition& other) const = default;
	std::string ToString() const;

	TextBufferPosition(const Utils::Point2D& pos, const TextChar& textChar, 
		const Font& font, const float& fontSize);
};

using TextBufferMixed = std::vector<TextBufferPosition>;
class TextBuffer
{
	FontData m_FontData;
	std::vector<TextCharPosition> m_TextPositions;

	TextBuffer(const FontData& font, const std::vector<TextCharPosition>& positions);
};

std::string ToString(const TextBufferMixed& buffer);

//template<typename T>
//struct TexturePosition
//{
//	Texture2D m_Texture;
//	T m_Pos;
//};
//
//struct WorldTextureBuffer
//{
//	std::vector<TexturePosition<WorldPosition>> m_TexturePositions;
//};
//
//struct ScreenTextureBuffer
//{
//	std::vector<TexturePosition<ScreenPosition>> m_TexturePositions;
//};


