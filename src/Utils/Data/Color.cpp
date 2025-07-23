#include "Utils/Data/Color.hpp"
#include <format>

namespace Utils
{
	std::string Color::ToString() const
	{
		return std::format("[Color R:{} G:{} B:{} A:{}]", m_R, m_G, m_B, m_A);
	}
}
