#include "pch.hpp"
#include "Core/UI/UIPadding.hpp"

namespace Engine::UI
{
	UIPadding::UIPadding(const float& left, const float& right, const float& top, const float& bottom)
		: m_Left(left), m_Right(right), m_Top(top), m_Bottom(bottom) {}

	void UIPadding::SetAll(const float value)
	{
		m_Left = value;
		m_Right = value;
		m_Top = value;
		m_Bottom = value;
	}
	bool UIPadding::HasNoPadding() const
	{
		return m_Left == 0.0f && m_Right == 0.0f && m_Top == 0.0f && m_Bottom == 0.0f;
	}

	std::string UIPadding::ToString() const
	{
		return std::format("[T:{} B:{} L:{} R:{}]", m_Top.ToString(),
			m_Bottom.ToString(), m_Left.ToString(), m_Right.ToString());
	}
}
