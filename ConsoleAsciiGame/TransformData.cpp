//NOT USED
#include "pch.hpp"

#include "TransformData.hpp"

TransformData::TransformData() : TransformData(Utils::Point2DInt{}) {}

TransformData::TransformData(const Utils::Point2DInt& pos) :
	m_Pos(pos), m_LastPos(NULL_POS), m_LastFramePos(NULL_POS), m_Dirty(false)
{

}