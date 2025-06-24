#pragma once
#include <string>
#include "Utils/HelperMacros.hpp"
#include "Core/Analyzation/Debug.hpp"

class NormalizedValue
{
private:
	float m_value;
public:
	NormalizedValue();
	NormalizedValue(const float value);

	float GetValue() const;
	void SetValue(const float value);

	std::string ToString() const;

	NormalizedValue operator-(const NormalizedValue& other) const;
	NormalizedValue operator+(const NormalizedValue& other) const;
	NormalizedValue operator*(const NormalizedValue& other) const;
	NormalizedValue operator*(const float& scalar) const;
	NormalizedValue operator/(const NormalizedValue& other) const;
	NormalizedValue operator/(const float& scalar) const;

	DECLARE_COMPOUND_ASSIGNMENT_OPERATORS(NormalizedValue)

	bool operator==(const NormalizedValue& other) const;
	bool operator!=(const NormalizedValue& other) const;
	bool operator>(const NormalizedValue& other) const;
	bool operator>=(const NormalizedValue& other) const;
	bool operator<(const NormalizedValue& other) const;
	bool operator<=(const NormalizedValue& other) const;

	//operator float() const;
};

