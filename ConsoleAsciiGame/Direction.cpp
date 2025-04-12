#include "pch.hpp"
#include "Direction.hpp"
#include "Debug.hpp"
#include "StringUtil.hpp"

bool IsAngledDirection(const MoveDirection& dir)
{
	return dir == MoveDirection::Northeast || dir == MoveDirection::Northwest || 
		   dir == MoveDirection::Southeast || dir == MoveDirection::Southwest;
}

Vec2 GetVectorFromDirection(const MoveDirection& dir)
{
	if (dir == MoveDirection::North) return Vec2::NORTH;
	else if (dir == MoveDirection::South) return Vec2::SOUTH;
	else if (dir == MoveDirection::East) return Vec2::EAST;
	else if (dir == MoveDirection::West) return Vec2::WEST;
	else if (dir == MoveDirection::Northeast) return Vec2::NORTHEAST;
	else if (dir == MoveDirection::Northwest) return Vec2::NORTHWEST;
	else if (dir == MoveDirection::Southeast) return Vec2::SOUTHEAST;
	else if (dir == MoveDirection::Southwest) return Vec2::SOUTHWEST;

	LogError(std::format("Tried to get vector from direction but no actions could be found"));
	return {};
}

std::string ToString(const MoveDirection& dir)
{
	if (dir == MoveDirection::North) return "North";
	else if (dir == MoveDirection::South) return "South";
	else if (dir == MoveDirection::East) return "East";
	else if (dir == MoveDirection::West) return "West";
	else if (dir == MoveDirection::Northeast) return "NorthEast";
	else if (dir == MoveDirection::Northwest) return "NorthWest";
	else if (dir == MoveDirection::Southeast) return "SouthEast";
	else if (dir == MoveDirection::Southwest) return "SouthWest"; 

	LogError(std::format("Tried to convert direction to string but no actions could be found"));
	return "";
}
std::optional<MoveDirection> TryConvertStringToDirection(const std::string& str, const bool& ignoreAngledDirs)
{
	std::string formattedStr = Utils::StringUtil(str).ToLowerCase().ToString();
	if (formattedStr == "north") return MoveDirection::North;
	else if (formattedStr == "south") return MoveDirection::South;
	else if (formattedStr == "east") return MoveDirection::East;
	else if (formattedStr == "west") return MoveDirection::West;

	if (ignoreAngledDirs) return std::nullopt;

	else if (formattedStr == "northeast") return MoveDirection::Northeast;
	else if (formattedStr == "northwest") return MoveDirection::Northwest;
	else if (formattedStr == "southeast") return MoveDirection::Southeast;
	else if (formattedStr == "southwest") return MoveDirection::Southwest;

	return std::nullopt;
}

std::optional<MoveDirection> TryConvertVectorToDirection(const Vec2& vec, const bool& ignoreAngledDirs)
{
	if (vec == Vec2::NORTH) return MoveDirection::North;
	else if (vec == Vec2::SOUTH) return MoveDirection::South;
	else if (vec == Vec2::EAST) return MoveDirection::East;
	else if (vec == Vec2::WEST) return MoveDirection::West;

	if (ignoreAngledDirs) return std::nullopt;

	const float dirAsAngle = vec.GetAngle(AngleMode::Degrees);
	if (0 < dirAsAngle && dirAsAngle < 90) return MoveDirection::Northeast;
	else if (90 < dirAsAngle && dirAsAngle < 180) return MoveDirection::Northwest;
	else if (180 < dirAsAngle && dirAsAngle < 270) return MoveDirection::Southwest;
	else if (270 < dirAsAngle && dirAsAngle < 360) return MoveDirection::Southeast;

	return std::nullopt;
}