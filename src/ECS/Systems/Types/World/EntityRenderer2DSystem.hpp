#pragma once
#include <string>
#include <vector>
#include <optional>
#include "Core/Visual/TextBuffer.hpp"
#include "Core/Primitives/Array2DPosition.hpp"
#include "ECS/Component/Component.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"
#include "ECS/Component/Types/World/EntityRenderer2DComponent.hpp"

namespace Engine::Rendering
{
	class Renderer;
	class EntityRenderer2DSystem
	{
	private:
		Rendering::Renderer* m_renderer;
	public:

	private:
		void AddTextToRenderer(EntityRenderer2DComponent& data, const Camera::CameraComponent& mainCamer);

	public:
		EntityRenderer2DSystem(Rendering::Renderer& renderer);

		std::string GetVisualString(const EntityRenderer2DComponent& data) const;

		void SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime);
	};
}


