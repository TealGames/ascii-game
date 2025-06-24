#include "pch.hpp"
#include "Core/Rendering/LineBuffer.hpp"

LineData::LineData(const WorldPosition& startPos, const WorldPosition& endPos) : 
	m_StartPos(startPos), m_EndPos(endPos) {}
