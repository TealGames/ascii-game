//NOT USED
#include "pch.hpp"

#include "Rect.hpp"
#include "Point2DInt.hpp"

Rect::Rect(const Utils::Point2DInt& topLeftPos, const Utils::Point2DInt& size) :
	m_topLeftPos(topLeftPos), m_size(size), m_bottomRightPos(m_topLeftPos) {}