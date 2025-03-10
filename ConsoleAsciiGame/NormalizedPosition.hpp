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
	void SetPos(const Vec2& relativePos);
};

