#pragma once
#include "Point2DInt.hpp"

//Since negative positions are not allowed
const Utils::Point2DInt NULL_POS = Utils::Point2DInt{ -1, -1 };

struct TransformData
{
	Utils::Point2DInt m_Pos;
	Utils::Point2DInt m_LastPos;
	Utils::Point2DInt m_LastFramePos;
	bool m_Dirty;

	TransformData();
	TransformData(const Utils::Point2DInt& pos);
};