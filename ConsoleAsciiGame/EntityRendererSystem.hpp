#pragma once
#include <string>
#include <vector>
#include <optional>
#include "raylib.h"
#include "TextBuffer.hpp"
#include "Array2DPosition.hpp"
#include "Component.hpp"
#include "TransformSystem.hpp"
#include "EntityRendererData.hpp"
#include "MultiBodySystem.hpp"

namespace ECS
{
	//struct VisualData
	//{
	//	//The position of the char data relative to the position of the character
	//	Utils::Point2DInt CharRelativePos;
	//	TextChar CharData;
	//};

	class EntityRendererSystem : MultiBodySystem
	{
	private:
		TransformSystem m_transformSystem;

	public:

	private:
		void RenderInBuffer(TextBuffer& buffer, EntityRendererData& data, const Entity& entity);

	public:
		EntityRendererSystem(const TransformSystem& transform);

		Array2DPosition GetGlobalVisualPos(const Array2DPosition& relativeVisualPos,
			const EntityRendererData& data, const Entity& entity) const;

		const std::vector<std::vector<TextChar>>& GetVisualData(const EntityRendererData& data) const;
		std::string GetVisualString(const EntityRendererData& data) const;

		void SystemUpdate(Scene& scene, const float& deltaTime) override;
	};
}


