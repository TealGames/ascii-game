#include "Utils/Data/Color.hpp"
#include <format>

namespace Utils
{
	Vec4 Color::GetNormalized() const
	{
		return
		{
			m_R / 255.0f,
			m_G / 255.0f,
			m_B / 255.0f,
			m_A / 255.0f,
		};
	}

	std::string Color::ToString() const
	{
		return std::format("[Color R:{} G:{} B:{} A:{}]", m_R, m_G, m_B, m_A);
	}

	Color GetColorFromHex(const std::uint32_t& hexNumber)
	{
		return
		{
			//We can just move the corresponding rgba value and then mask it
			//to only include that segment, and convert to unsigned char as needed
			static_cast<std::uint8_t>((hexNumber >> 24) & 0xFF),
			static_cast<std::uint8_t>((hexNumber >> 16) & 0xFF),
			static_cast<std::uint8_t>((hexNumber >> 8) & 0xFF),
			static_cast<std::uint8_t>(hexNumber & 0xFF)
		};
	}

}
