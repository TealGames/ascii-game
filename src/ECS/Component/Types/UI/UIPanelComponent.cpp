#include "pch.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"
#include "Utils/Debug.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/World/EntityComponent.hpp"
#include "Core/Rendering/Renderer3d.hpp"

UIPanelComponent::UIPanelComponent() : UIPanelComponent(HDRColor()) {}
UIPanelComponent::UIPanelComponent(const HDRColor color) : m_color(color), m_renderer(nullptr) {}

void UIPanelComponent::SetColor(const HDRColor color)
{
	m_color = color;
}

void UIPanelComponent::Render(const float depth, const Mat3& globalModelMatrix)
{
	//TODO: add ui render call
	m_renderer->GetRendererMutable().AddCallRect2D(m_color, nullptr, depth, globalModelMatrix);
}

void UIPanelComponent::InitFields()
{
	m_Fields = {};
}
std::string UIPanelComponent::ToString() const
{
	return std::format("[PanelGUI color:{}]", m_color.ToString());
}

void UIPanelComponent::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json UIPanelComponent::Serialize()
{
	//TODO: implement
	return {};
}
