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
}
