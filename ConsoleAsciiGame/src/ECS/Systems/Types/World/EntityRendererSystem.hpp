#pragma once
#include <string>
#include <vector>
#include <optional>
#include "Core/Visual/TextBuffer.hpp"
#include "Utils/Data/Array2DPosition.hpp"
#include "ECS/Component/Component.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"
#include "ECS/Component/Types/World/EntityRendererData.hpp"
#include "ECS/Systems/MultiBodySystem.hpp"

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


