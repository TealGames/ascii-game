#include "pch.hpp"
#include "NormalizedValue.hpp"

NormalizedValue::NormalizedValue() : NormalizedValue(0) {}
NormalizedValue::NormalizedValue(const float value) 
	: m_value() 
{
	SetValue(value);
}

float NormalizedValue::GetValue() const
{
	return m_value;
}
void NormalizedValue::SetValue(const float value)
{
	m_value= std::clamp(value, float(0), float(1));
}

std::string NormalizedValue::ToString() const
{
	return std::to_string(m_value);
}

NormalizedValue NormalizedValue::operator-(const NormalizedValue& other) const
{
	return NormalizedValue(m_value - other.m_value);
}
NormalizedValue NormalizedValue::operator+(const NormalizedValue& other) const
{
	return NormalizedValue(m_value + other.m_value);
}
NormalizedValue NormalizedValue::operator*(const NormalizedValue& other) const
{
	return NormalizedValue(m_value * other.m_value);
}
NormalizedValue NormalizedValue::operator*(const float& scalar) const
{
	return NormalizedValue(m_value * scalar);
}
NormalizedValue NormalizedValue::operator/(const NormalizedValue& other) const
{
	if (other.m_value == 0)
	{
		Assert(false, std::format("Attempted to divide a normalized value:{} "
			"by a zero value:{}", ToString(), other.ToString()));
		return *this;
	}
	return NormalizedValue(m_value / other.m_value);
}
NormalizedValue NormalizedValue::operator/(const float& scalar) const
{
	if (scalar == 0)
	{
		Assert(false, std::format("Attempted to divide a normalized value:{} "
			"by a zero scalar:{}", ToString(), std::to_string(scalar)));
		return *this;
	}

	return NormalizedValue(m_value / scalar);
}

IMPLEMENT_COMPOUND_ASSIGNMENT_OPERATORS(NormalizedValue)

bool NormalizedValue::operator==(const NormalizedValue& other) const
{
	return Utils::ApproximateEqualsF(m_value, other.m_value);
}
bool NormalizedValue::operator!=(const NormalizedValue& other) const 
{
	return !(*this == other);
}
bool NormalizedValue::operator>(const NormalizedValue& other) const
{
	return m_value > other.m_value;
}
bool NormalizedValue::operator>=(const NormalizedValue& other) const
{
	return *this > other || *this == other;
}
bool NormalizedValue::operator<(const NormalizedValue& other) const
{
	return *this < other;
}
bool NormalizedValue::operator<=(const NormalizedValue& other) const
{
	return *this < other || *this == other;
}