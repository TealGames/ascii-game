#pragma once
#include "Utils/Data/Vec2.hpp"
#include <optional>
#include <string>

enum class MoveDirection
{
	North,
	Northeast,
	East,
	Southeast,
	South,
	Southwest,
	West,
	Northwest,
};

bool IsAngledDirection(const MoveDirection& dir);

Vec2 GetVectorFromDirection(const MoveDirection& dir);
std::optional<MoveDirection> TryConvertVectorToDirection(const Vec2& vec, const bool& ignoreAngledDirs=false);

std::string ToString(const MoveDirection& dir);
std::optional<MoveDirection> TryConvertStringToDirection(const std::string& str, const bool& ignoreAngledDirs=false);