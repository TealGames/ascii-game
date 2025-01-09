//NOT USED
#include "pch.hpp"

#include "EntityRendererData.hpp"

EntityRendererData::EntityRendererData() : EntityRendererData(std::vector<std::vector<TextChar>>{}, RenderLayerType::None) {}

EntityRendererData::EntityRendererData(const std::vector<std::vector<TextChar>>& visualData, const RenderLayerType& renderLayers) :
	m_VisualData(visualData), m_LastFrameVisualData(), m_VisualBoundsSize(), m_RenderLayers(renderLayers)
{
	int maxWidth = 0;
	for (const auto& row : m_VisualData)
	{
		if (row.size() > maxWidth) maxWidth = row.size();
	}
	m_VisualBoundsSize = Utils::Point2DInt(m_VisualData.size(), maxWidth);
}