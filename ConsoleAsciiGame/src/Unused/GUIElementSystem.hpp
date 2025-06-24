#pragma once
#include "ECS/Systems/MultiBodySystem.hpp"

namespace ECS
{
	class GUIElementSystem : public MultiBodySystem
	{
	private:
	public:

	private:
	public:
		GUIElementSystem();
		void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) override;
	};
}


