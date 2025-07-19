#pragma once
#include "Utils/Data/NormalizedValue.hpp"

struct RelativeUIPadding
{
	NormalizedValue m_Left;
	NormalizedValue m_Right;
	NormalizedValue m_Top;
	NormalizedValue m_Bottom;

	RelativeUIPadding(const float& left = 0, const float& right = 0, const float& top = 0, const float& bottom = 0);

	void SetAll(const float value);
	bool HasNonZeroPadding() const;

	std::string ToString() const;
};

