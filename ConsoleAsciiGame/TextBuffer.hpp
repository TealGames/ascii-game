#pragma once
#include "TextArray.hpp"
#include "Vec2.hpp"
#include "FontData.hpp"

//using TextBuffer = TextArray;

//TODO: perhaps the camera or something should group together into a new structure with the same font and font size
//so that we do not need to repeat the same data for mutliple entries
struct TextBufferPosition
{
	Vec2 m_Pos;
	FontData m_FontData;
	TextChar m_Text;

	bool operator==(const TextBufferPosition& other) const = default;
	std::string ToString() const;

	TextBufferPosition(const Vec2& pos, const TextChar& textChar, 
		const Font& font, const float& fontSize);

	TextBufferPosition(const Vec2& pos, const TextChar& textChar,
		const FontData& fontData);
};

using TextBufferMixed = std::vector<TextBufferPosition>;
class TextBuffer
{
	FontData m_FontData;
	std::vector<TextCharArrayPosition> m_TextPositions;

	TextBuffer(const FontData& font, const std::vector<TextCharArrayPosition>& positions);
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


