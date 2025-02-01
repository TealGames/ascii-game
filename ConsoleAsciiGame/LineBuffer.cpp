#include "pch.hpp"
#include "LineBuffer.hpp"

LineData::LineData(const WorldPosition& startPos, const WorldPosition& endPos) : 
	m_StartPos(startPos), m_EndPos(endPos) {}
