#include "pch.hpp"
#include "PanelGUI.hpp"
#include "Debug.hpp"
#include "RaylibUtils.hpp"

PanelGUI::PanelGUI() : PanelGUI(Color()) {}
PanelGUI::PanelGUI(const Color color) : m_color(color) {}

void PanelGUI::Update(const float deltaTime) {}
RenderInfo PanelGUI::Render(const RenderInfo& renderInfo)
{
	DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y,
		renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y, m_color);
	//Assert(false, std::format("Rendering panel gui:{} color:{}", renderInfo.ToString(), RaylibUtils::ToString(m_color)));
	return renderInfo;
}
