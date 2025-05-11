#pragma once
#include "Vec2.hpp"

/// <summary>
/// Represents a normalized position of a rectangle where [0,0] would represent 
/// bottom left and [1,1] would represent top right where it is in [WIDTH, HEIGHT] [X, Y]
/// </summary>
class NormalizedPosition
{
private:
	Vec2 m_pos;
public:
	static const Vec2 TOP_LEFT;
	static const Vec2 TOP_RIGHT;
	static const Vec2 BOTTOM_LEFT;
	static const Vec2 BOTTOM_RIGHT;
	static const Vec2 CENTER;
	static const Vec2 BOTTOM_CENTER;
	static const Vec2 TOP_CENTER;

private:
	bool IsValidPos() const;
public:
	NormalizedPosition();
	NormalizedPosition(const float& x, const float& y);
	NormalizedPosition(const Vec2& pos);

	const Vec2& GetPos() const;
	/// <summary>
	/// Note: this should rarely be used since it can bypass the position
	/// setting from the public set function
	/// </summary>
	/// <returns></returns>
	Vec2& GetPosMutable();

	const float& GetX() const;
	const float& GetY() const;

	float& GetXMutable();
	float& GetYMutable();

	void SetPos(const Vec2& relativePos);

	bool IsZero() const;

	NormalizedPosition operator+(const NormalizedPosition& other) const;
	NormalizedPosition operator-(const NormalizedPosition& other) const;
	NormalizedPosition operator*(const NormalizedPosition& other) const;
	NormalizedPosition operator*(const float& scalar) const;
	NormalizedPosition operator/(const float& scalar) const;

	bool operator==(const NormalizedPosition& other) const;
	bool operator!=(const NormalizedPosition& other) const;
	bool operator>(const NormalizedPosition& other) const;
	bool operator>=(const NormalizedPosition& other) const;
	bool operator<(const NormalizedPosition& other) const;
	bool operator<=(const NormalizedPosition& other) const;
};

NormalizedPosition GetSizeFromCorners(const NormalizedPosition& topLeft, const NormalizedPosition& bottomRight);
NormalizedPosition GetBottomRight(const NormalizedPosition& topLeft, const NormalizedPosition& size);
NormalizedPosition GetTopLeft(const NormalizedPosition& bottomRight, const NormalizedPosition& size);

