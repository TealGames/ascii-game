#include "pch.hpp"
#include "Core/Primitives/NormalizedValue.hpp"
#include "Utils/Math.hpp"
#include "Utils/Debug.hpp"

namespace Engine
{
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
		m_value = std::clamp(value, 0.0f, 1.0f);
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
		ENGINE_ASSERT(other != 0, "Attempted to divide a normalized value:{} "
			"by a zero value:{}", ToString(), other.ToString());
		return NormalizedValue(m_value / other.m_value);
	}
	NormalizedValue NormalizedValue::operator/(const float& other) const
	{
		ENGINE_ASSERT(!::Math::ApproximateEqualsF(other, 0.0f), "Attempted to divide a normalized value:{} "
			"by a zero scalar value:{}", ToString(), other);
		return NormalizedValue(m_value / other);
	}

	NormalizedValue& NormalizedValue::operator+=(const NormalizedValue& other)
	{
		SetValue(m_value + other.m_value);
		return *this;
	}
	NormalizedValue& NormalizedValue::operator-=(const NormalizedValue& other)
	{
		SetValue(m_value - other.m_value);
		return *this;
	}
	NormalizedValue& NormalizedValue::operator*=(const NormalizedValue& other)
	{
		SetValue(m_value * other.m_value);
		return *this;
	}
	NormalizedValue& NormalizedValue::operator/=(const NormalizedValue& other)
	{
		ENGINE_ASSERT(other != 0, "Attempted to divide a normalized value:{} "
			"by a zero value:{}", ToString(), other.ToString());
		SetValue(m_value / other.m_value);
		return *this;
	}

	bool NormalizedValue::operator==(const NormalizedValue& other) const
	{
		return ::Math::ApproximateEqualsF(m_value, other.m_value);
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
		return m_value < other.m_value;
	}
	bool NormalizedValue::operator<=(const NormalizedValue& other) const
	{
		return *this < other || *this == other;
	}
}
