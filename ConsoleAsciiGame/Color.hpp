#pragma once
#include <cstdint>
#include <string>

namespace Utils
{
	struct Color
	{
		std::uint8_t m_R;
		std::uint8_t m_G;
		std::uint8_t m_B;
		std::uint8_t m_A;

		Color();
		Color(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b);
		Color(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a);

		std::string ToString() const;
	};
}


