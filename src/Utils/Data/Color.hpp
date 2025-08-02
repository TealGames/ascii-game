#pragma once
#include <cstdint>
#include <string>

namespace Utils
{
	inline constexpr std::uint8_t DEFAULT_CHANNEL_VALUE = 255;
	struct Color
	{
		std::uint8_t m_R;
		std::uint8_t m_G;
		std::uint8_t m_B;
		std::uint8_t m_A;

		constexpr Color() : Color(DEFAULT_CHANNEL_VALUE, DEFAULT_CHANNEL_VALUE, DEFAULT_CHANNEL_VALUE, DEFAULT_CHANNEL_VALUE) {}
		constexpr Color(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b) : Color(r, g, b, DEFAULT_CHANNEL_VALUE) {}
		constexpr Color(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a)
			: m_R(r), m_G(g), m_B(b), m_A(a) {}

		constexpr Color(const Color rColor, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a) :
			Color(rColor.m_R, g, b, a) {}
		constexpr Color(const Color rgColor, const std::uint8_t b, const std::uint8_t a) :
			Color(rgColor.m_R, rgColor.m_G, b, a) {}
		constexpr Color(const Color rgbColor, const std::uint8_t a) :
			Color(rgbColor.m_R, rgbColor.m_G, rgbColor.m_B, a) {}

		auto operator<=>(const Color&) const = default;

		std::string ToString() const;
	};

	Color GetColorFromHex(const std::uint32_t& hexNumber);
}




