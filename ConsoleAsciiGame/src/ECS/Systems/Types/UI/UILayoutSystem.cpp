#include "pch.hpp"
#include "ECS/Systems/Types/UI/UILayoutSystem.hpp"
#include "ECS/Systems/MultiBodySystem.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"

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

