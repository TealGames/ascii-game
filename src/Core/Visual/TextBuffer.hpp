#pragma once
#include "Core/Primitives/WorldPosition.hpp"
#include "Core/Visual/TextArray.hpp"
#include "Core/Rendering/FontData.hpp"

namespace Engine::Rendering
{
	//TODO: perhaps the camera or something should group together into a new structure with the same font and font size
//so that we do not need to repeat the same data for mutliple entries
	struct TextBufferCharPosition2D
	{
		WorldPosition2D m_Pos;
		TextChar m_Text;
		WorldFontProperties m_FontData;

		TextBufferCharPosition2D();
		TextBufferCharPosition2D(const WorldPosition2D& pos, const TextChar& textChar, const WorldFontProperties& font);
		Vec2 GetWorldSize() const;

		std::string ToString() const;
	};
	std::string ToString(const std::vector<TextBufferCharPosition2D>& chars);

	struct TextBufferCharPosition3D
	{
		WorldPosition3D m_Pos;
		TextChar m_Text;
		WorldFontProperties m_FontData;

		TextBufferCharPosition3D();
		TextBufferCharPosition3D(const WorldPosition3D& pos, const TextChar& textChar, const WorldFontProperties& font);
		Vec2 GetWorldSize() const;

		std::string ToString() const;
	};

	struct TextBufferChar
	{
		TextChar m_Text;
		WorldFontProperties m_FontData;

		TextBufferChar();
		TextBufferChar(const TextChar& textChar, const WorldFontProperties& font);
		Vec2 GetWorldSize() const;

		std::string ToString() const;
	};

	using FragmentedTextBuffer2D = std::vector<TextBufferCharPosition2D>;
	using FragmentedTextBuffer3D = std::vector<TextBufferCharPosition3D>;

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

}


