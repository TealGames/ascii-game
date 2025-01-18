//NOT USED
#include "pch.hpp"

#include "EntityRendererData.hpp"

EntityRendererData::EntityRendererData() : EntityRendererData(VisualData{}, RenderLayerType::None) {}

EntityRendererData::EntityRendererData(const VisualData& visualData, const RenderLayerType& renderLayers) :
	ComponentData(), m_VisualData(visualData), m_LastFrameVisualData(), m_VisualBoundsSize(), m_RenderLayers(renderLayers)
{
	int maxWidth = 0;
	for (const auto& row : m_VisualData.GetFull())
	{
		if (row.size() > maxWidth) maxWidth = row.size();
	}
	m_VisualBoundsSize = Utils::Point2DInt(m_VisualData.GetHeight(), maxWidth);
}