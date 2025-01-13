#pragma once
#include "Point2DInt.hpp"
#include "ComponentData.hpp"

//Since negative positions are not allowed
const Utils::Point2DInt NULL_POS = Utils::Point2DInt{ -1, -1 };

struct TransformData : public ComponentData
{
	Utils::Point2DInt m_Pos;
	Utils::Point2DInt m_LastPos;
	Utils::Point2DInt m_LastFramePos;

	TransformData();
	TransformData(const Utils::Point2DInt& pos);
};