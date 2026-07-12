#include "pch.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "Core/Rendering/Renderer3d.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::UI
{
	UIRendererComponent::UIRendererComponent() : Component(), m_renderer(nullptr), m_lastRenderArea() {}

	Rendering::Renderer& UIRendererComponent::GetRendererMutable()
	{
		return *m_renderer;
	}
	const UIRect& UIRendererComponent::GetLastRenderRect() const
	{
		return m_lastRenderArea;
	}

	void UIRendererComponent::InitFields()
	{
		m_Fields = {};
	}
	void UIRendererComponent::Serialize(Serialization::Serializer& serializer) const
	{
	}
	void UIRendererComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
	}
	std::string UIRendererComponent::ToString() const
	{
		return std::format("[UIRenderer]");
	}
}
