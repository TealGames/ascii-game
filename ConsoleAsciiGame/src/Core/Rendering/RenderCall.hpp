#pragma once
#include <variant>
#include "Utils/Data/Vec2.hpp"
#include "Utils/Data/Vec2Int.hpp"
#include "Utils/Data/ScreenPosition.hpp"
#include "Utils/Data/Color.hpp"
#include "raylib.h"

namespace Rendering
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
		ScreenPosition m_Pos;
		float m_Radius;
		Color m_Color;
	};
	struct RectCall
	{
		ScreenPosition m_Pos;
		Vec2 m_Size;
		Color m_Color;
	};

	using TextureID = std::uint16_t;
	struct TextureCall
	{
		TextureID m_Id;
		ScreenPosition m_Pos;
		Color m_Color;
	};
	struct TextureCallData
	{
		Texture2D m_Tex;
		Vec2 m_Scale;
	};

	using TextID = std::uint16_t;
	struct TextCall
	{
		TextID m_Id;
		ScreenPosition m_Pos;
		Color m_Color;
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
		ScreenPosition m_Pos;
		float m_Thickness;
		Vec2 m_Length;
		Color m_Color;
	};
	struct RectLineCall
	{
		ScreenPosition m_Pos;
		float m_Thickness;
		Vec2 m_Size;
		Color m_Color;
	};

	using RenderCall = std::variant<CircleCall, RectCall, TextureCall, TextCall, LineCall, RectLineCall>;
}
