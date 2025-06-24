#pragma once
#include <variant>
#include "Vec2.hpp"
#include "ScreenPosition.hpp"
#include "Color.hpp"
#include "raylib.h"

namespace Rendering
{
	struct CircleCall
	{
		ScreenPosition m_Pos;
		float m_Radius;
		Color m_Color;
	};
	struct RectCall
	{
		ScreenPosition m_Pos;
		Vec2Int m_Size;
		Color m_Color;
	};

	using TextureID = std::uint16_t;
	struct TextureCall
	{
		ScreenPosition m_Pos;
		TextureID m_Id;
		Color m_Color;
	};
	struct TextureCallData
	{
		Texture2D m_Tex;
		float m_Rotation;
		float m_Scale;
	};

	using TextID = std::uint16_t;
	struct TextCall
	{
		ScreenPosition m_Pos;
		TextID m_Id;
		Color m_Color;
	};
	struct TextCallData
	{
		Font m_Font;
		const char* m_Text;
		float m_FontSize;
		float m_Spacing;
	};

	using RenderCall = std::variant<CircleCall, RectCall, TextureCall, TextCall>;
}
