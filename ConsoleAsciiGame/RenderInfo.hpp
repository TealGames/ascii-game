#pragma once
#include "ScreenPosition.hpp"

struct RenderInfo
{
	ScreenPosition m_TopLeftPos;
	ScreenPosition m_RenderSize;

	RenderInfo();
	RenderInfo(const ScreenPosition& topLeftPos, const ScreenPosition& renderSize);

	std::string ToString() const;
};