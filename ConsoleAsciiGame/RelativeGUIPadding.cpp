#include "pch.hpp"
#include "RelativeGUIPadding.hpp"


RelativeGUIPadding::RelativeGUIPadding(const float& left, const float& right, const float& top, const float& bottom) 
	: m_Left(left), m_Right(right), m_Top(top), m_Bottom(bottom) {}

void RelativeGUIPadding::SetAll(const float value)
{
	m_Left = value;
	m_Right = value;
	m_Top = value;
	m_Bottom = value;
}
bool RelativeGUIPadding::HasNonZeroPadding() const
{
	return m_Left != 0 || m_Right != 0 || m_Top != 0 || m_Bottom != 0;
}

std::string RelativeGUIPadding::ToString() const
{
	return std::format("[T:{} B:{} L:{} R:{}]", m_Top.ToString(), 
		m_Bottom.ToString(), m_Left.ToString(), m_Right.ToString());
}