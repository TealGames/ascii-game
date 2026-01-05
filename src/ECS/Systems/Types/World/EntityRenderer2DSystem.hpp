#pragma once
#include <string>
#include <vector>
#include <optional>
#include "Core/Visual/TextBuffer.hpp"
#include "Utils/Data/Array2DPosition.hpp"
#include "ECS/Component/Component.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"
#include "ECS/Component/Types/World/EntityRenderer2DComponent.hpp"

namespace Rendering { class Renderer; }
namespace ECS
{
	class EntityRenderer2DSystem
	{
	private:
		Rendering::Renderer* m_renderer;
	public:

	private:
		void AddTextToRenderer(EntityRenderer2DComponent& data, const CameraComponent& mainCamer);

	public:
		EntityRenderer2DSystem(Rendering::Renderer& renderer);

		std::string GetVisualString(const EntityRenderer2DComponent& data) const;

		void SystemUpdate(Scene& scene, CameraComponent& mainCamera, const float& deltaTime);
	};
}


