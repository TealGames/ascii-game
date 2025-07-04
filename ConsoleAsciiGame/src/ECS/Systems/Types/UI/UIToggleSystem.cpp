#include "pch.hpp"
#include "UIToggleSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/UI/UISelectableData.hpp"
#include "ECS/Systems/MultiBodySystem.hpp"

namespace ECS
{
	UIToggleSystem::UIToggleSystem() {}
	void UIToggleSystem::Init()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIToggleComponent),
			ComponentInfo(CreateComponentTypes<UISelectableData>(), CreateRequiredComponentFunction(UISelectableData()),
				[](EntityData& entity)-> void
				{
					entity.TryGetComponentMutable<UIToggleComponent>()->m_selectable = entity.TryGetComponentMutable<UISelectableData>();
				}));
	}
}
