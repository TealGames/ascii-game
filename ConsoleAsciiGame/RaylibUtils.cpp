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

	Color MultiplyColorsRGBA(const Color& color, const Utils::Point4D& factor)
	{
		unsigned char newR = color.r * factor.m_X;
		unsigned char newG = color.g * factor.m_Y;
		unsigned char newB = color.b * factor.m_Z;
		unsigned char newA = color.a * factor.m_W;
		return {newR, newG, newB, newA};
	}

	Color MultiplyColorsRGB(const Color& color, const Utils::Point3D& factor)
	{
		unsigned char newR = color.r * factor.m_X;
		unsigned char newG = color.g * factor.m_Y;
		unsigned char newB = color.b * factor.m_Z;
		return { newR, newG, newB, color.a};
	}

	Utils::Point4D DivideColorRGBA(const Color& divided, const Color& divisor)
	{
		return Utils::Point4D((float)divided.r / divisor.r, (float)divided.g / divisor.g, (float)divided.b / divisor.b, (float)divided.a / divisor.a);
	}

	Utils::Point3D DivideColorRGB(const Color& divided, const Color& divisor)
	{
		return Utils::Point3D((float)divided.r / divisor.r, (float)divided.g / divisor.g, (float)divided.b / divisor.b);
	}

	Utils::Point4D GetFractionalColorRGBA(const Color& color, const float& multiplier)
	{
		return {color.r* multiplier , color.g*multiplier, color.b*multiplier, color.a*multiplier};
	}

	Utils::Point3D GetFractionalColorRGB(const Color& color, const float& multiplier)
	{
		return { color.r * multiplier , color.g * multiplier, color.b * multiplier};
	}

	Color GetColorFromPoint(const Utils::Point4D& rgba)
	{
		return { static_cast<unsigned char>(rgba.m_X),
				 static_cast<unsigned char>(rgba.m_Y),
				 static_cast<unsigned char>(rgba.m_Z),
				 static_cast<unsigned char>(rgba.m_W) };
	}

	Color GetColorFromPoint(const Utils::Point3D& rgb)
	{
		return { static_cast<unsigned char>(rgb.m_X),
				 static_cast<unsigned char>(rgb.m_Y),
				 static_cast<unsigned char>(rgb.m_Z) };
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
