#include "pch.hpp"
#include "UIRendererComponent.hpp"
#include "GameRenderer.hpp"

UIRendererData::UIRendererData() : Component(), m_renderer(nullptr) {}

void UIRendererData::SetRenderer(Rendering::Renderer& renderer)
{
	m_renderer = &renderer;
}
Rendering::Renderer& UIRendererData::GetRendererMutable()
{
	return *m_renderer;
}

void UIRendererData::SetLastRenderRect(const GUIRect& rect)
{
	m_lastRenderRect = rect;
}
const GUIRect& UIRendererData::GetLastRenderRect() const
{
	return m_lastRenderRect;
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