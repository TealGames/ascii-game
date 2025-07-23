#pragma once

struct UIPadding
{
	float m_Left;
	float m_Right;
	float m_Top;
	float m_Bottom;

	constexpr UIPadding(const float left=0, const float right=0, const float top=0, const float bottom=0) :
		m_Left(left), m_Right(right), m_Top(top), m_Bottom(bottom) {}
};

