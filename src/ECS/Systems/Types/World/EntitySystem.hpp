#pragma once
#include "ECS/Systems/MultiBodySystem.hpp"

namespace ECS
{
	class EntitySystem : MultiBodySystem
	{
	private:
	public:

	private:
	public:
		EntitySystem();
		void SystemUpdate(Scene& scene, CameraComponent& mainCamera, const float& deltaTime) override;
	};
}


