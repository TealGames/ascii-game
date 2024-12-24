#pragma once
#include "Point2DInt.hpp"

class Rect
{
private:
	Utils::Point2DInt m_topLeftPos;
	Utils::Point2DInt m_bottomRightPos;
	Utils::Point2DInt m_size;

public:

private:
public:
	Rect(const Utils::Point2DInt& topLeftPos, const Utils::Point2DInt& size);
};

