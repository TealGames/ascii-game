#include "pch.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"
#include "Utils/Debug.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/Rendering/Renderer3d.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::UI
{
	UIPanelComponent::UIPanelComponent() : UIPanelComponent(ColHDR4()) {}
	UIPanelComponent::UIPanelComponent(const ColHDR4 color) : m_color(color), m_renderer(nullptr) {}

	void UIPanelComponent::SetColor(const ColHDR4 color)
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
	void UIPanelComponent::Serialize(Serialization::Serializer& serializer) const
	{
	}
	void UIPanelComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
	}
	std::string UIPanelComponent::ToString() const
	{
		return std::format("[PanelGUI color:{}]", m_color.ToString());
	}
}

