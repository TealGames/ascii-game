#include "pch.hpp"
#include "RenderInfo.hpp"

RenderInfo::RenderInfo() : m_TopLeftPos(), m_RenderSize() { }

RenderInfo::RenderInfo(const ScreenPosition& topLeftPos, const ScreenPosition& renderSize)
	: m_TopLeftPos(topLeftPos), m_RenderSize(renderSize) {}

std::string RenderInfo::ToString() const
{
	return std::format("[RenderInfo TLPos:{} Size:{}]", m_TopLeftPos.ToString(), m_RenderSize.ToString());
}