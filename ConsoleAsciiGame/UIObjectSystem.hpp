#pragma once
#include "MultiBodySystem.hpp"
#include "UIObjectData.hpp"

namespace ECS
{
	class UIObjectSystem : MultiBodySystem
	{
	private:
	public:

	private:
	public:
		UIObjectSystem();
		void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) override;
	};
}

