#pragma once
#include <vector>
#include "Point2DInt.hpp"
#include "RenderLayer.hpp"
#include "ComponentData.hpp"
#include "VisualData.hpp"

class EntityRendererData : public ComponentData
{
private:
	//Stores every entity's visual from top row to bottom row
	//TODO: the enttiy render data should store the layer it should render at, but not the buffer itself
	RenderLayerType m_RenderLayers;

	//The bounding box's size for this visual in (HEIGHT, WIDTH)
	Utils::Point2DInt m_VisualBoundsSize;

public:
	VisualData m_VisualData;
	std::vector<TextCharPosition> m_LastFrameVisualData;
	
public:
	EntityRendererData();
	EntityRendererData(const VisualData& visualData, const RenderLayerType& renderLayers);

	const RenderLayerType& GetRenderLayers() const;
	const Utils::Point2DInt& GetVisualBoundsSize() const;
	const VisualData& GetVisualData() const;
};