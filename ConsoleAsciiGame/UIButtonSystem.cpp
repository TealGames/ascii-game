#include "pch.hpp"
#include "UIButtonSystem.hpp"
#include "MultiBodySystem.hpp"
#include "Scene.hpp"

namespace ECS
{
	UIButtonSystem::UIButtonSystem() {}

	void UIButtonSystem::SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime)
	{
		scene.OperateOnComponents<UIButton>(
			[this, &scene, &deltaTime](UIButton& data)-> void
			{

			});
	}
}

