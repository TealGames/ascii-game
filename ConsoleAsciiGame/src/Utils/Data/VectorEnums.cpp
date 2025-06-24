#include "pch.hpp"
#include "VectorEnums.hpp"
#include "Debug.hpp"

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

std::string ToString(const VectorForm& form)
{
	if (form == VectorForm::Component) return "Component";
	else if (form == VectorForm::MagnitudeDirection) return "Magnitude@Direction";
	else if (form == VectorForm::Unit) return "Unit";
	else
	{
		LogError("Tried to convert undefined Vec2Int form mode to string");
		return "";
	}
}
