#include "pch.hpp"
#include "ECS/Systems/Types/UI/UILayoutSystem.hpp"
#include "ECS/Component/Types/UI/UILayoutComponent.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"

namespace Engine::UI
{
	UILayoutSystem::UILayoutSystem() {}

	void UILayoutSystem::SystemUpdate(Scenes::GlobalEntityManager& globalEntityManager, const float& deltaTime)
	{
		globalEntityManager.OperateOnComponents<UILayoutComponent>(ECS::ALL_ACTIVE_ENABLED_FLAG,
			[&deltaTime](UILayoutComponent& data)-> void
			{
				data.Update(deltaTime);
			});
	}
}

