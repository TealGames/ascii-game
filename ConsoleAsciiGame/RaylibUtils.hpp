#pragma once
#include <string>
#include <format>
#include <cstdint>
#include <limits>
#include "raylib.h"
#include "HelperFunctions.hpp"
#include "Point4D.hpp"
#include "Point3D.hpp"

namespace RaylibUtils
{
	std::string ToString(const Color& color);
	Color MultiplyColorsRGBA(const Color& color, const Utils::Point4D& factor);
	Color MultiplyColorsRGB(const Color& color, const Utils::Point3D& factor);
	Utils::Point4D DivideColorRGBA(const Color& divided, const Color& divisor);
	Utils::Point3D DivideColorRGB(const Color& divided, const Color& divisor);

	Utils::Point4D GetFractionalColorRGBA(const Color& color, const float& multiplier);
	Utils::Point3D GetFractionalColorRGB(const Color& color, const float& multiplier);

	Color GetColorFromPoint(const Utils::Point4D& rgba);
	Color GetColorFromPoint(const Utils::Point3D& rgb);

	Color GetColorFromHex(const unsigned int& hexNumber);
}

