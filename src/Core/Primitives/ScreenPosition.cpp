#include "Core/Primitives/ScreenPosition.hpp"
#include "StaticGlobals.hpp"

namespace Engine
{
	ScreenPosition NormalizedScreenToPosition(const NormalizedVec2& normalized)
	{
		Vec2 screenValues = { normalized.AsVec2().m_X * SCREEN_WIDTH, normalized.AsVec2().m_Y * SCREEN_HEIGHT };
		return ScreenPosition{ screenValues.m_X, SCREEN_HEIGHT - screenValues.m_Y };
	}

	NormalizedVec2 ScreenToNormalizedPosition(const ScreenPosition& pos, const Vec2Int totalScreenArea)
	{
		return { static_cast<float>(pos.m_X) / totalScreenArea.m_X, float(1) - (static_cast<float>(pos.m_Y) / totalScreenArea.m_Y) };
	}
}