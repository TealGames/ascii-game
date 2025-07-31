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

namespace Rendering { class Renderer; }
namespace ECS
{
	class EntityRendererSystem : MultiBodySystem
	{
	private:
		Rendering::Renderer* m_renderer;
	public:

	private:
		void AddTextToRenderer(EntityRendererData& data, const CameraComponent& mainCamer);

	public:
		EntityRendererSystem(Rendering::Renderer& renderer);

		std::string GetVisualString(const EntityRendererData& data) const;

		void SystemUpdate(Scene& scene, CameraComponent& mainCamera, const float& deltaTime) override;
	};
}


