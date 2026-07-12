#include "pch.hpp"
#include "UIToggleSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/UI/UISelectableComponent.hpp"
#include "ECS/Component/Types/UI/UIToggleComponent.hpp"
#include "ECS/Systems/MultiBodySystem.hpp"

namespace Engine::UI
{
	UIToggleSystem::UIToggleSystem() {}
	void UIToggleSystem::Init()
	{
		ECS::GlobalComponentInfo::AddComponentInfo(typeid(UIToggleComponent),
			ECS::ComponentInfo(ECS::CreateComponentTypes<UISelectableComponent>(), CreateRequiredComponentFunction(UISelectableComponent()),
				[](ECS::EntityData& entity)-> void
				{
					UIToggleComponent& toggle = *(entity.TryGetComponentMutable<UIToggleComponent>());
					toggle.m_selectable = entity.TryGetComponentMutable<UISelectableComponent>();
					toggle.Init();
				}));
	}
}
