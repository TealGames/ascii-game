#include "Utils/Data/Enums.hpp"
#include "Utils/Debug.hpp"

std::string ToString(const AngleMode& mode)
{
	if (mode == AngleMode::Degrees) return "Degrees";
	else if (mode == AngleMode::Radians) return "Radians";
	else
	{
		LogError("Tried to convert undefined Vector2 angle mode to string");
		return "";
	}
}