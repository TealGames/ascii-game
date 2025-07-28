#include "pch.hpp"
#include "Core/Rendering/LineBuffer.hpp"

LineData::LineData(const WorldPosition3D& startPos, const WorldPosition3D& endPos) : 
	m_StartPos(startPos), m_EndPos(endPos) {}
