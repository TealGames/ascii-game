#include "RaylibUtils.hpp"
#include <string>
#include <format>
#include "raylib.h"
#include "HelperFunctions.hpp"

namespace RaylibUtils
{
	std::string ToString(const Color& color)
	{
		return std::format("[R:{}, G:{}, B:{}, A:{}]",
			std::to_string(color.r), std::to_string(color.g), std::to_string(color.b), std::to_string(color.a));
	}
}
