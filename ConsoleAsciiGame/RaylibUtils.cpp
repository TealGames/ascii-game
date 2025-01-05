#include "pch.hpp"
#include "raylib.h"
#include "RaylibUtils.hpp"

namespace RaylibUtils
{
	std::string ToString(const Color& color)
	{
		return std::format("[R:{}, G:{}, B:{}, A:{}]",
			std::to_string(color.r), std::to_string(color.g), std::to_string(color.b), std::to_string(color.a));
	}

	Color GetColorFromHex(const std::uint32_t& hexNumber)
	{
		return
		{
			//We can just move the corresponding rgba value and then mask it
			//to only include that segment, and convert to unsigned char as needed
			static_cast<unsigned char>((hexNumber >> 24) & 0xFF),
			static_cast<unsigned char>((hexNumber >> 16) & 0xFF),
			static_cast<unsigned char>((hexNumber >> 8) & 0xFF), 
			static_cast<unsigned char>(hexNumber & 0xFF)  
		};
	}
}
