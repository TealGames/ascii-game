#pragma once
#include <variant>
#include "Core/Primitives/WorldPosition.hpp"
#include "Core/Primitives/Color.hpp"
#include "Core/Rendering/Texture.hpp"
#include "Core/Rendering/Font.hpp"
//#include "raylib.h"

namespace Engine::Rendering
{
	enum class RenderShape : std::uint8_t
	{
		Circle			= 0,
		Rectangle		= 1,
		Texture			= 2,
		Text			= 3,
		Line			= 4,
		RectangleLine	= 5,
		Ray				= 6
	};

	struct CircleCall
	{
		WorldPosition3D m_Pos;
		float m_Radius;
		ColHDR4 m_Color;
	};
	struct RectCall
	{
		WorldPosition3D m_Pos;
		Vec2 m_Size;
		ColHDR4 m_Color;
	};

	using TextureID = std::uint16_t;
	struct TextureCall
	{
		TextureID m_Id;
		WorldPosition3D m_Pos;
		ColHDR4 m_Color;
	};
	struct TextureCallData
	{
		Texture m_Tex;
		Vec2 m_Scale;
	};

	using TextID = std::uint16_t;
	struct TextCall
	{
		TextID m_Id;
		WorldPosition3D m_Pos;
		ColHDR4 m_Color;
	};
	struct TextCallData
	{
		Font m_Font;
		const char* m_Text;
		float m_FontSize;
		float m_Spacing;
	};

	struct LineCall
	{
		WorldPosition3D m_Pos;
		float m_Thickness;
		Vec2 m_Length;
		ColHDR4 m_Color;
	};
	struct RectLineCall
	{
		WorldPosition3D m_Pos;
		float m_Thickness;
		Vec2 m_Size;
		ColHDR4 m_Color;
	};

	using RenderCall = std::variant<CircleCall, RectCall, TextureCall, TextCall, LineCall, RectLineCall>;
}
