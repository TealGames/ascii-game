#include "pch.hpp"
#include "UIRendererComponent.hpp"
#include "GameRenderer.hpp"

UIRendererData::UIRendererData() : Component(), m_renderer(nullptr), m_lastRenderRect(), m_isEventBlocker(false) {}

Rendering::Renderer& UIRendererData::GetRendererMutable()
{
	return *m_renderer;
}

const UIRect& UIRendererData::GetLastRenderRect() const
{
	return m_lastRenderRect;
}

void UIRendererData::SetEventBlocker(const bool status)
{
	m_isEventBlocker = status;
}
bool UIRendererData::IsSelectionEventBlocker() const
{
	return m_isEventBlocker;
}

void UIRendererData::InitFields()
{
	m_Fields = {};
}
std::string UIRendererData::ToString() const
{
	return std::format("[UIRenderer]");
}

void UIRendererData::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json UIRendererData::Serialize()
{
	//TODO: implement
	return {};
}