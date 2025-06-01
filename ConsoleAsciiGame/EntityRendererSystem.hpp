#pragma once
#include <string>
#include <vector>
#include <optional>
#include "TextBuffer.hpp"
#include "Array2DPosition.hpp"
#include "Component.hpp"
#include "TransformSystem.hpp"
#include "EntityRendererData.hpp"
#include "MultiBodySystem.hpp"

namespace ECS
{
	class EntityRendererSystem : MultiBodySystem
	{
	private:

	public:

	private:
		void AddTextToBuffer(FragmentedTextBuffer& buffer, EntityRendererData& data);

	public:
		EntityRendererSystem();

		std::string GetVisualString(const EntityRendererData& data) const;

		void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) override;
	};
}


