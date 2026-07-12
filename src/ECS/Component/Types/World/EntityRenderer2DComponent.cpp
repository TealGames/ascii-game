#include "pch.hpp"
#include "Core/Serialization/Serializer.hpp"
#include "ECS/Component/Types/World/EntityRenderer2DComponent.hpp"
#include "Utils/Debug.hpp"

namespace Engine::Rendering
{
	EntityRenderer2DComponent::EntityRenderer2DComponent() : EntityRenderer2DComponent(VisualData(), RenderLayerType::None) {}

	EntityRenderer2DComponent::EntityRenderer2DComponent(const VisualData& visualData, const RenderLayerType& renderLayers) :
		Component(), m_VisualData(visualData), m_renderLayers(renderLayers)//, m_LastFrameVisualData(),
	{

	}

	RenderLayerType EntityRenderer2DComponent::GetRenderLayers() const
	{
		return m_renderLayers;
	}

	const VisualData& EntityRenderer2DComponent::GetVisualData() const
	{
		return m_VisualData;
	}

	void EntityRenderer2DComponent::OverrideVisualData(const VisualData& newVisual)
	{
		m_VisualData = newVisual;
	}

	void EntityRenderer2DComponent::InitFields()
	{
		m_Fields = {};
	}
	void EntityRenderer2DComponent::Serialize(Serialization::Serializer& serializer) const
	{
		serializer.AddProperty("Layers", m_renderLayers);
		serializer.AddProperty("VisualData", m_VisualData);
	}
	void EntityRenderer2DComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
		deserializer.GetProperty("Layers", &m_renderLayers);
		deserializer.GetProperty("VisualData", &m_VisualData);
	}

	std::string EntityRenderer2DComponent::ToString() const
	{
		return std::format("[EntityRenderer Layers:{} Visual:{}]",
			Rendering::ToString(m_renderLayers), m_VisualData.ToString());
	}
}
