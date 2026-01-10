#pragma once
#include <string>
#include "Utils/HelperMacros.hpp"

class NormalizedValue
{
private:
	float m_value;
public:
	static constexpr float MIN = 0;
	static constexpr float HALF = 0.5;
	static constexpr float MAX = 1;

	NormalizedValue();
	NormalizedValue(const float value);
	
	float GetValue() const;
	void SetValue(const float value);

	std::string ToString() const;

	NormalizedValue operator+(const NormalizedValue& other) const;
	NormalizedValue operator-(const NormalizedValue& other) const;
	NormalizedValue operator*(const NormalizedValue& other) const;
	NormalizedValue operator/(const NormalizedValue& other) const;
	NormalizedValue operator*(const float& scalar) const;
	NormalizedValue operator/(const float& scalar) const;

	NormalizedValue& operator+=(const NormalizedValue& other);
	NormalizedValue& operator-=(const NormalizedValue& other);
	NormalizedValue& operator*=(const NormalizedValue& other);
	NormalizedValue& operator/=(const NormalizedValue& other);

	bool operator==(const NormalizedValue& other) const;
	bool operator!=(const NormalizedValue& other) const;
	bool operator>(const NormalizedValue& other) const;
	bool operator>=(const NormalizedValue& other) const;
	bool operator<(const NormalizedValue& other) const;
	bool operator<=(const NormalizedValue& other) const;

	//operator float() const;
};

