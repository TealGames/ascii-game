#pragma once
#include <vector>
#include "Point2DInt.hpp"
#include "RenderLayer.hpp"
#include "ComponentData.hpp"
#include "VisualData.hpp"

struct EntityRendererData : ComponentData
{
	//Stores every entity's visual from top row to bottom row
	//TODO: the enttiy render data should store the layer it should render at, but not the buffer itself
	VisualData m_VisualData;
	RenderLayerType m_RenderLayers;
	std::vector<TextCharPosition> m_LastFrameVisualData;

	//The bounding box's size for this visual in (HEIGHT, WIDTH)
	Utils::Point2DInt m_VisualBoundsSize;
	
	EntityRendererData();
	EntityRendererData(const VisualData& visualData, const RenderLayerType& renderLayers);
};