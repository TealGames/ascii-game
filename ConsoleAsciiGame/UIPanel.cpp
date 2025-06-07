#include "pch.hpp"
#include "UIPanel.hpp"
#include "Debug.hpp"
#include "RaylibUtils.hpp"
#include "UIRendererComponent.hpp"
#include "EntityData.hpp"
#include "GameRenderer.hpp"

UIPanel::UIPanel() : UIPanel(Color()) {}
UIPanel::UIPanel(const Color color) : m_color(color) {}

void UIPanel::SetColor(const Color color)
{
	m_color = color;
}

GUIRect UIPanel::Render(const GUIRect& renderInfo)
{
	UIRendererData* renderer = GetEntitySafeMutable().TryGetComponentMutable<UIRendererData>();
	renderer->GetRendererMutable().AddRectangleCall(renderInfo.m_TopLeftPos, renderInfo.GetSize(), m_color);

	return renderInfo;
}

void UIPanel::InitFields()
{
	m_Fields = {};
}
std::string UIPanel::ToString() const
{
	return std::format("[PanelGUI color:{}]", RaylibUtils::ToString(m_color));
}

void UIPanel::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json UIPanel::Serialize()
{
	//TODO: implement
	return {};
}
