#pragma once
#include "Utils/Data/Vec2Type.hpp"
#include <optional>
#include <string>
#include "Utils/Data/DirectionEnums.hpp"

bool IsAngledDirection(const MoveDirection& dir);

Vec2 GetVectorFromDirection(const MoveDirection& dir);
std::optional<MoveDirection> TryConvertVectorToDirection(const Vec2& vec, const bool& ignoreAngledDirs=false);

std::string ToString(const MoveDirection& dir);
std::optional<MoveDirection> TryConvertStringToDirection(const std::string& str, const bool& ignoreAngledDirs=false);