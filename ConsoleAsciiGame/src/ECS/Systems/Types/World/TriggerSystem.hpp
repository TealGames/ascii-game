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

		void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) override;
	};
}


