#include "pch.hpp"
#include "FloatRange.hpp"
#include "HelperFunctions.hpp"

FloatRange::FloatRange() : FloatRange(0, 0) {}

FloatRange::FloatRange(const float& min, const float& max) 
	: m_Min(min), m_Max(max) {}

float FloatRange::GetRandom() const
{
	return static_cast<float>(Utils::GenerateRandomDouble(m_Min, m_Max));
}

std::string FloatRange::ToString() const
{
	return std::format("[Min:{}, Max:{}]", 
		std::to_string(m_Min), std::to_string(m_Max));
}
