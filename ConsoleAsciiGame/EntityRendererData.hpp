#pragma once
#include <vector>
#include "Vec2Int.hpp"
#include "RenderLayer.hpp"
#include "ComponentData.hpp"
#include "VisualData.hpp"
#include "IJsonSerializable.hpp"

class EntityRendererData : public ComponentData, public IJsonSerializable<EntityRendererData>
{
private:
	//Stores every entity's visual from top row to bottom row
	//TODO: the enttiy render data should store the layer it should render at, but not the buffer itself
	RenderLayerType m_renderLayers;

	//The bounding box's size for this visual in (WIDTH, HEIGHT)
	//TODO: this is unused
	Vec2Int m_visualBoundsSize;

public:
	VisualData m_VisualData;
	std::vector<TextCharPosition> m_LastFrameVisualData;
	
public:
	EntityRendererData();
	EntityRendererData(const Json& json);
	EntityRendererData(const VisualData& visualData, const RenderLayerType& renderLayers);

	const RenderLayerType& GetRenderLayers() const;
	const Vec2Int& GetVisualBoundsSize() const;
	const VisualData& GetVisualData() const;

	void SetVisualData(const VisualDataPositions& positions);

	void InitFields() override;

	std::string ToString() const override;

	EntityRendererData& Deserialize(const Json& json) override;
	Json Serialize(const EntityRendererData& component) override;
};