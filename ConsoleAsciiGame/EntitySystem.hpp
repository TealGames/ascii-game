#pragma once
#include "MultiBodySystem.hpp"

namespace ECS
{
	class EntitySystem : MultiBodySystem
	{
	private:
	public:

	private:
	public:
		EntitySystem();
		void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) override;
	};
}


