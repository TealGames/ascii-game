#pragma once

struct FloatRange
{
	float m_Min;
	float m_Max;

	FloatRange(const float& min, const float& max);

	float GetRandom() const;
};

