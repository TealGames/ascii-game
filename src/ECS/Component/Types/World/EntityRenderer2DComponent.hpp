#pragma once
#include <vector>
#include "Core/Primitives/Vector.hpp"
#include "Core/Rendering/RenderLayer.hpp"
#include "ECS/Component/Component.hpp"
#include "Core/Visual/VisualData.hpp"

namespace Engine::Rendering
{
	class EntityRenderer2DComponent : public ECS::Component
	{
	private:
		//Stores every entity's visual from top row to bottom row
		//TODO: the enttiy render data should store the layer it should render at, but not the buffer itself
		RenderLayerType m_renderLayers;

	public:
		VisualData m_VisualData;
		//std::vector<TextCharArrayPosition> m_LastFrameVisualData;

	public:
		EntityRenderer2DComponent();
		EntityRenderer2DComponent(const VisualData& visualData, const RenderLayerType& renderLayers);

		RenderLayerType GetRenderLayers() const;
		//Vec2Int GetVisualSize() const;
		const VisualData& GetVisualData() const;

		//void SetVisualDataDeltas(const VisualDataPositions& positions);
		void OverrideVisualData(const VisualData& newVisual);

		//std::vector<std::string> GetDependencyFlags() const override;
		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}
