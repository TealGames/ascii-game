#include "pch.hpp"
#include "UILayoutSystem.hpp"
#include "MultiBodySystem.hpp"
#include "GlobalEntityManager.hpp"

namespace ECS
{
	UILayoutSystem::UILayoutSystem() {}

	void UILayoutSystem::SystemUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime)
	{
		globalEntityManager.OperateOnComponents<UILayout>(
			[this, &deltaTime](UILayout& data)-> void
			{
				data.Update(deltaTime);
			});
	}
}

