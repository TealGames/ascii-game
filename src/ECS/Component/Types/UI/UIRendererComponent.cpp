#include "pch.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "Core/Rendering/GameRenderer.hpp"

UIRendererData::UIRendererData() : Component(), m_renderer(nullptr), m_lastRenderArea() {}

Rendering::Renderer& UIRendererData::GetRendererMutable()
{
	return *m_renderer;
}
const UIRect& UIRendererData::GetLastRenderRect() const
{
	return m_lastRenderArea;
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