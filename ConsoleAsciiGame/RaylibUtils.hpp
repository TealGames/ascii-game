#pragma once
#include <string>
#include <format>
#include "raylib.h"
#include "HelperFunctions.hpp"

namespace RaylibUtils
{
	std::string ToString(const Color& color);
	Color GetColorFromHex(const unsigned int& hexNumber);
}

