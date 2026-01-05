#include "pch.hpp"
#include "Core/Serialization/JsonSerializers.hpp"
#include "ECS/Component/Types/World/EntityRenderer2DComponent.hpp"
#include "Core/Serialization/JsonUtils.hpp"
#include "Utils/Debug.hpp"

EntityRenderer2DComponent::EntityRenderer2DComponent() : EntityRenderer2DComponent(VisualData(), RenderLayerType::None) {}

EntityRenderer2DComponent::EntityRenderer2DComponent(const Json& json) : EntityRenderer2DComponent()
{
	Deserialize(json);
}

EntityRenderer2DComponent::EntityRenderer2DComponent(const VisualData& visualData, const RenderLayerType& renderLayers) :
	Component(), m_VisualData(visualData), m_renderLayers(renderLayers)//, m_LastFrameVisualData(),
{

}

RenderLayerType EntityRenderer2DComponent::GetRenderLayers() const
{
	return m_renderLayers;
}
//Vec2Int EntityRendererData::GetVisualSize() const
//{
//	return m_VisualData.GetBufferSize();
//}

const VisualData& EntityRenderer2DComponent::GetVisualData() const
{
	return m_VisualData;
}

//void EntityRendererData::SetVisualDataDeltas(const VisualDataPositions& positions)
//{
//	for (const auto& pos : positions)
//	{
//		LogError(std::format("Attempting to set the pos:{} for entity renderer of {} full visual:{}", 
//			pos.ToString(), GetEntitySafe().GetName(), ToString()));
//		m_VisualData.m_Text.SetAt(pos.m_RowColPos, pos.m_Text);
//	}
//}
void EntityRenderer2DComponent::OverrideVisualData(const VisualData& newVisual)
{
	m_VisualData = newVisual;
}

void EntityRenderer2DComponent::InitFields()
{
	m_Fields = {};
}
//std::vector<std::string> EntityRendererData::GetDependencyFlags() const
//{
//	return {};
//}

std::string EntityRenderer2DComponent::ToString() const
{
	return std::format("[EntityRenderer Layers:{} Visual:{}]", 
		::ToString(m_renderLayers), m_VisualData.ToString());
}

void EntityRenderer2DComponent::Deserialize(const Json& json)
{
	m_renderLayers = json.at("Layers").get<RenderLayerType>();
	/*Assert(false, std::format("Deserialiazed layers: {} to: {} json:{} TOSTZRING: {}", ::ToString(json.at("Layers").get<RenderLayerType>()), 
		::ToString(m_renderLayers), JsonUtils::ToStringProperties(json), ToString()));*/
	m_VisualData = json.at("VisualData").get<VisualData>();
}
Json EntityRenderer2DComponent::Serialize()
{
	return { {"Layers", m_renderLayers}, {"VisualData", m_VisualData}};
}