#include "pch.hpp"
#include "JsonSerializers.hpp"
#include "EntityRendererData.hpp"

EntityRendererData::EntityRendererData() : EntityRendererData(VisualData{}, RenderLayerType::None) {}

EntityRendererData::EntityRendererData(const Json& json) : EntityRendererData()
{
	Deserialize(json);
}

EntityRendererData::EntityRendererData(const VisualData& visualData, const RenderLayerType& renderLayers) :
	ComponentData(), m_VisualData(visualData), 
	m_LastFrameVisualData(), m_visualBoundsSize(), m_renderLayers(renderLayers)
{
	int maxWidth = 0;
	for (const auto& row : m_VisualData.m_Text.GetFull())
	{
		if (row.size() > maxWidth) maxWidth = row.size();
	}
	m_visualBoundsSize = Vec2Int(maxWidth, m_VisualData.m_Text.GetHeight());
}

const RenderLayerType& EntityRendererData::GetRenderLayers() const
{
	return m_renderLayers;
}
const Vec2Int& EntityRendererData::GetVisualBoundsSize() const
{
	return m_visualBoundsSize;
}

const VisualData& EntityRendererData::GetVisualData() const
{
	return m_VisualData;
}

void EntityRendererData::SetVisualData(const VisualDataPositions& positions)
{
	for (const auto& pos : positions.m_Data)
	{
		m_VisualData.m_Text.SetAt(pos.m_RowColPos, pos.m_Text);
	}
}

void EntityRendererData::InitFields()
{
	m_Fields = {};
}

std::string EntityRendererData::ToString() const
{
	return std::format("[EntityRenderer Layers:{} Visual:{}]", 
		::ToString(m_renderLayers), m_VisualData.ToString());
}

EntityRendererData& EntityRendererData::Deserialize(const Json& json)
{
	m_renderLayers = json.at("Layers").get<RenderLayerType>();
	m_VisualData = json.at("VisualData").get<VisualData>();
	return *this;
}
Json EntityRendererData::Serialize(const EntityRendererData& component)
{
	return { {"Layers", m_renderLayers}, {"VisualData", m_VisualData}};
}