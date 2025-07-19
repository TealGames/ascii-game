#pragma once
#include <vector>
#include "Utils/Data/Vec2Int.hpp"
#include "Core/Rendering/RenderLayer.hpp"
#include "ECS/Component/Component.hpp"
#include "Core/Visual/VisualData.hpp"

class EntityRendererData : public Component
{
private:
	//Stores every entity's visual from top row to bottom row
	//TODO: the enttiy render data should store the layer it should render at, but not the buffer itself
	RenderLayerType m_renderLayers;

public:
	VisualData m_VisualData;
	//std::vector<TextCharArrayPosition> m_LastFrameVisualData;
	
public:
	EntityRendererData();
	EntityRendererData(const Json& json);
	EntityRendererData(const VisualData& visualData, const RenderLayerType& renderLayers);

	RenderLayerType GetRenderLayers() const;
	//Vec2Int GetVisualSize() const;
	const VisualData& GetVisualData() const;

	//void SetVisualDataDeltas(const VisualDataPositions& positions);
	void OverrideVisualData(const VisualData& newVisual);

	//std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};