#pragma once
#include "Utils/Data/Vec2.hpp"
#include "Core/Visual/TextArray.hpp"
#include "Core/Rendering/FontData.hpp"

//TODO: perhaps the camera or something should group together into a new structure with the same font and font size
//so that we do not need to repeat the same data for mutliple entries
struct TextBufferCharPosition
{
	Vec2 m_Pos;
	TextChar m_Text;
	FontProperties m_FontData;

	TextBufferCharPosition();
	TextBufferCharPosition(const Vec2& pos, const TextChar& textChar, const FontProperties& font);
	Vec2 GetWorldSize() const;

	std::string ToString() const;
};
std::string ToString(const std::vector<TextBufferCharPosition>& chars);

struct TextBufferChar
{
	TextChar m_Text;
	FontProperties m_FontData;

	TextBufferChar();
	TextBufferChar(const TextChar& textChar, const FontProperties& font);
	Vec2 GetWorldSize() const;

	std::string ToString() const;
};

using FragmentedTextBuffer = std::vector<TextBufferCharPosition>;

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


