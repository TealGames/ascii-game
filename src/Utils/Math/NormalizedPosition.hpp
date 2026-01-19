#pragma once
#include "Utils/Math/Vec2Type.hpp"
#include "Utils/Math/NormalizedValue.hpp"

/// <summary>
/// Represents a normalized coordinate where x is in range [0,1] 
/// and y is in range [0,1]
/// </summary>
class NormalizedPos
{
private:
public:
	NormalizedValue m_X;
	NormalizedValue m_Y;

private:
	//bool IsValidPos() const;
public:

	NormalizedPos();
	NormalizedPos(const float& x, const float& y);
	NormalizedPos(const Vec2& pos);
	NormalizedPos(const NormalizedValue x, const NormalizedValue y);
	NormalizedPos(const float x, const NormalizedValue y);
	NormalizedPos(const NormalizedValue x, const float y);

	float GetX() const;
	float GetY() const;
	Vec2 AsVec2() const;

	void SetPos(const Vec2& relativePos);
	//void SetPosX(const float x);
	//void SetPosY(const float y);
	//void SetPosDeltaX(const float deltaX);
	//void SetPosDeltaY(const float deltaY);
	bool IsZero() const;

	std::string ToString() const;

	NormalizedPos operator+(const NormalizedPos& other) const;
	NormalizedPos operator-(const NormalizedPos& other) const;
	NormalizedPos operator*(const NormalizedPos& other) const;
	NormalizedPos operator/(const NormalizedPos& other) const;
	NormalizedPos operator*(const float& scalar) const;
	NormalizedPos operator/(const float& scalar) const;

	NormalizedPos& operator+=(const NormalizedPos& other);
	NormalizedPos& operator-=(const NormalizedPos& other);

	bool operator==(const NormalizedPos& other) const;
	bool operator!=(const NormalizedPos& other) const;
	bool operator>(const NormalizedPos& other) const;
	bool operator>=(const NormalizedPos& other) const;
	bool operator<(const NormalizedPos& other) const;
	bool operator<=(const NormalizedPos& other) const;
};

