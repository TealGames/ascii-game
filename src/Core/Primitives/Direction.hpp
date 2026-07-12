#pragma once
#include <optional>
#include "Core/Primitives/Vector.hpp"

namespace Engine
{
	enum class MoveDirection : std::uint8_t
	{
		North			= 0,
		Northeast		= 1,
		East			= 2,
		Southeast		= 3,
		South			= 4,
		Southwest	    = 5,
		West			= 6,
		Northwest		= 7
	};

	bool IsAngledDirection(const MoveDirection& dir);

	Engine::Vec2 GetVectorFromDirection(const MoveDirection& dir);
	std::optional<MoveDirection> TryConvertVectorToDirection(const Vec2& vec, const bool& ignoreAngledDirs = false);

	std::string ToString(const MoveDirection& dir);
	std::optional<MoveDirection> TryConvertStringToDirection(const std::string& str, const bool& ignoreAngledDirs = false);
}
