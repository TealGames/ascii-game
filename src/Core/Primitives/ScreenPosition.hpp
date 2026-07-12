#pragma once
#include "Core/Primitives/NormalizedVec2.hpp"

namespace Engine
{
	//A position relative to the screen for raylib of the form [X, Y] where (0,0) is top left
	//and bottom right is the canvas (WIDTH, HEIGHT)
	using ScreenPosition = Vec2;
	constexpr ScreenPosition INVALID_SCREEN_POS = { -1, -1 };

	ScreenPosition NormalizedScreenToPosition(const NormalizedVec2& pos);
	NormalizedVec2 ScreenToNormalizedPosition(const ScreenPosition& pos, const Vec2Int totalScreenArea);

	enum class CoordinateOriginType : std::uint8_t
	{
		/// <summary>
		/// y INCREASES DOWNWARD (V) and x INCREASES RIGHTWARD (>)
		/// </summary>
		TopLeft = 0,
		/// <summary>
		/// y INCREASES UPWARD (^) and x INCREASES RIGHTWARD (>)
		/// </summary>
		BottomLeft = 1,
	};

	template<typename T>
	std::optional<Vec<T, 2>> TryToNewFixedAreaPos(const Vec<T, 2> fixedArea, const CoordinateOriginType startOriginType,
		const Vec<T, 2>& pos, const CoordinateOriginType newOriginType)
	{
		if (startOriginType == newOriginType)
			return pos;

		Vec<T, 2> newPos = pos;
		if ((startOriginType == CoordinateOriginType::BottomLeft && newOriginType == CoordinateOriginType::TopLeft) ||
			(startOriginType == CoordinateOriginType::TopLeft && newOriginType == CoordinateOriginType::BottomLeft))
		{
			newPos.m_Y = fixedArea.m_Y - pos.m_Y;
		}

		if (newPos == pos) return std::nullopt;
		return newPos;
	}
}
