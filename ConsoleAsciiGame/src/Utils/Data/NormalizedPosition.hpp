#pragma once
#include "Vec2.hpp"
#include "NormalizedValue.hpp"

/// <summary>
/// Represents a normalized position of a rectangle where [0,0] would represent 
/// bottom left and [1,1] would represent top right where it is in [WIDTH, HEIGHT] [X, Y]
/// </summary>
class NormalizedPosition
{
private:
public:
	static constexpr float MIN = 0;
	static constexpr float HALF = 0.5;
	static constexpr float MAX = 1;

	static const Vec2 TOP_LEFT;
	static const Vec2 TOP_RIGHT;
	static const Vec2 BOTTOM_LEFT;
	static const Vec2 BOTTOM_RIGHT;
	static const Vec2 CENTER;
	static const Vec2 BOTTOM_CENTER;
	static const Vec2 TOP_CENTER;

	NormalizedValue m_X;
	NormalizedValue m_Y;

private:
	//bool IsValidPos() const;
public:
	NormalizedPosition();
	NormalizedPosition(const float& x, const float& y);
	NormalizedPosition(const Vec2& pos);
	NormalizedPosition(const NormalizedValue x, const NormalizedValue y);
	NormalizedPosition(const float x, const NormalizedValue y);
	NormalizedPosition(const NormalizedValue x, const float y);

	const Vec2 GetPos() const;
	/// <summary>
	/// Note: this should rarely be used since it can bypass the position
	/// setting from the public set function
	/// </summary>
	/// <returns></returns>
	//Vec2& GetPosMutable();

	const float GetX() const;
	const float GetY() const;

	/*float& GetXMutable();
	float& GetYMutable();*/

	void SetPos(const Vec2& relativePos);
	//void SetPosX(const float x);
	//void SetPosY(const float y);
	//void SetPosDeltaX(const float deltaX);
	//void SetPosDeltaY(const float deltaY);
	bool IsZero() const;

	std::string ToString() const;

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

