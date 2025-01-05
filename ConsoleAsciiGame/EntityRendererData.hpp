#pragma once
#include <vector>
#include "Point2DInt.hpp"
#include "RenderLayer.hpp"

struct EntityRendererData
{
	//Stores every entity's visual from top row to bottom row
	//TODO: the enttiy render data should store the layer it should render at, but not the buffer itself
	const std::vector<std::vector<TextChar>> m_VisualData;
	const RenderLayerType m_RenderLayers;
	std::vector<TextCharPosition> m_LastFrameVisualData;

	//The bounding box's size for this visual in (HEIGHT, WIDTH)
	Utils::Point2DInt m_VisualBoundsSize;
	bool m_Dirty;
	
	EntityRendererData();
	EntityRendererData(const std::vector<std::vector<TextChar>>& visualData, const RenderLayerType& renderLayers);
};