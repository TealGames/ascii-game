#pragma once
#include "Utils/Data/Vec4Type.hpp"

namespace Utils
{
	inline constexpr std::uint8_t DEFAULT_CHANNEL_VALUE = 255;
	struct Color
	{
		std::uint8_t m_R;
		std::uint8_t m_G;
		std::uint8_t m_B;
		std::uint8_t m_A;

		constexpr Color() 
			: m_R(DEFAULT_CHANNEL_VALUE), m_G(DEFAULT_CHANNEL_VALUE), m_B(DEFAULT_CHANNEL_VALUE), m_A(DEFAULT_CHANNEL_VALUE) {}
		constexpr Color(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b) 
			: m_R(r), m_G(g), m_B(b), m_A(DEFAULT_CHANNEL_VALUE) {}
		constexpr Color(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a)
			: m_R(r), m_G(g), m_B(b), m_A(a) {}

		/*
		constexpr Color(const float r, const float g, const float b) 
			: m_R(std::clamp(r* DEFAULT_CHANNEL_VALUE, 0.0f, 1.0f)), 
			  m_G(std::clamp(g* DEFAULT_CHANNEL_VALUE, 0.0f, 1.0f)), 
			  m_B(std::clamp(b* DEFAULT_CHANNEL_VALUE, 0.0f, 1.0f)), 
			  m_A(DEFAULT_CHANNEL_VALUE) {}
		constexpr Color(const float r, const float g, const float b, const float a)
			: m_R(std::clamp(r* DEFAULT_CHANNEL_VALUE, 0.0f, 1.0f)),
			  m_G(std::clamp(g* DEFAULT_CHANNEL_VALUE, 0.0f, 1.0f)),
			  m_B(std::clamp(b* DEFAULT_CHANNEL_VALUE, 0.0f, 1.0f)),
			  m_A(std::clamp(a* DEFAULT_CHANNEL_VALUE, 0.0f, 1.0f)) {}
		*/

		constexpr Color(const Color rColor, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a)
			: m_R(rColor.m_R), m_G(g), m_B(b), m_A(a) {}
		constexpr Color(const Color rgColor, const std::uint8_t b, const std::uint8_t a)
			: m_R(rgColor.m_R), m_G(rgColor.m_G), m_B(b), m_A(a) {}
		constexpr Color(const Color rgbColor, const std::uint8_t a)
			: m_R(rgbColor.m_R), m_G(rgbColor.m_G), m_B(rgbColor.m_B), m_A(a) {}

		/*
		constexpr Color(const Color rColor, const float g, const float b, const float a)
			: m_R(rColor.m_R), m_G(std::clamp(g* DEFAULT_CHANNEL_VALUE, 0.0f, 1.0f)), 
			  m_B(std::clamp(b* DEFAULT_CHANNEL_VALUE, 0.0f, 1.0f)), 
			  m_A(std::clamp(a* DEFAULT_CHANNEL_VALUE, 0.0f, 1.0f)) {}
		constexpr Color(const Color rgColor, const float b, const float a)
			: m_R(rgColor.m_R), m_G(rgColor.m_G), 
			  m_B(std::clamp(b* DEFAULT_CHANNEL_VALUE, 0.0f, 1.0f)),
			  m_A(std::clamp(a* DEFAULT_CHANNEL_VALUE, 0.0f, 1.0f)) {}
		constexpr Color(const Color rgbColor, const float a)
			: m_R(rgbColor.m_R), m_G(rgbColor.m_G), m_B(rgbColor.m_B), 
			  m_A(std::clamp(a* DEFAULT_CHANNEL_VALUE, 0.0f, 1.0f)) {}
			  */

		/// <summary>
		/// Will get the color for each rgba value between 0 and 1
		/// </summary>
		/// <returns></returns>
		Vec4 GetNormalized() const;

		auto operator<=>(const Color&) const = default;

		std::string ToString() const;
	};

	Color GetColorFromHex(const std::uint32_t& hexNumber);
}




