#include "pch.hpp"
#include "Utils/Data/Color.hpp"
#include "Utils/HelperFunctions.hpp"

namespace Utils
{
	Color::Color() : Color(255, 255, 255, 255) {}
	Color::Color(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b) : Color(r, g, b, 0) {}
	Color::Color(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a)
		: m_R(r), m_G(g), m_B(b), m_A(a) {}

	std::string Color::ToString() const
	{
		return std::format("[Color R:{} G:{} B:{} A:{}]", Utils::ToString(m_R), 
			Utils::ToString(m_G), Utils::ToString(m_B), Utils::ToString(m_A));
	}
}
