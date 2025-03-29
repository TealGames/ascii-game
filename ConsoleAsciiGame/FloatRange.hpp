#pragma once
#include <string>

struct FloatRange
{
	float m_Min;
	float m_Max;

	FloatRange();
	FloatRange(const float& min, const float& max);

	float GetRandom() const;
	std::string ToString() const;
};

