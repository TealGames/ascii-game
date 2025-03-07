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

private:
public:
	NormalizedPosition();
	NormalizedPosition(const float& x, const float& y);
	NormalizedPosition(const Vec2& pos);

	const Vec2& GetPos() const;
	void SetPos(const Vec2& relativePos);
};

