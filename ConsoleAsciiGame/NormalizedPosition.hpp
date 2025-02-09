#pragma once
#include "Point2D.hpp"

/// <summary>
/// Represents a normalized position of a rectangle where [0,0] would represent 
/// bottom left and [1,1] would represent top right where it is in [WIDTH, HEIGHT] [X, Y]
/// </summary>
class NormalizedPosition
{
private:
	Utils::Point2D m_pos;
public:

private:
public:
	NormalizedPosition();
	NormalizedPosition(const float& x, const float& y);
	NormalizedPosition(const Utils::Point2D& pos);

	const Utils::Point2D& GetPos() const;
	void SetPos(const Utils::Point2D& relativePos);
};

