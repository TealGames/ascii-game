#pragma once
#include "ECS/Systems/MultiBodySystem.hpp"

namespace ECS
{
	class TriggerSystem : public MultiBodySystem
	{
	private:
	public:

	private:
	public:
		TriggerSystem();

		void SystemUpdate(Scene& scene, CameraComponent& mainCamera, const float& deltaTime) override;
	};
}


