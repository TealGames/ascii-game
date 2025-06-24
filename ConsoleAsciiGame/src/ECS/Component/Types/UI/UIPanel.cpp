#include "pch.hpp"
#include "ECS/Component/Types/UI/UIPanel.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Utils/RaylibUtils.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/Rendering/GameRenderer.hpp"

UIPanel::UIPanel() : UIPanel(Color()) {}
UIPanel::UIPanel(const Color color) : m_color(color), m_renderer(nullptr) {}

void UIPanel::SetColor(const Color color)
{
	m_color = color;
}

UIRect UIPanel::Render(const UIRect& renderInfo)
{
	m_renderer->GetRendererMutable().AddRectangleCall(renderInfo.m_TopLeftPos, renderInfo.GetSize(), m_color);
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
