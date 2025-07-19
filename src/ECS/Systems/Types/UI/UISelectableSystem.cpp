#include "pch.hpp"
#include "ECS/Systems/Types/UI/UISelectableSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

namespace ECS
{
	UISelectableSystem::UISelectableSystem() {}
	void UISelectableSystem::Init()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UISelectableData),
			ComponentInfo(CreateComponentTypes<UIRendererData>(), CreateRequiredComponentFunction(UIRendererData()),
				[](EntityData& entity)-> void
				{
					entity.TryGetComponentMutable<UISelectableData>()->m_renderer = entity.TryGetComponentMutable<UIRendererData>();
				}));
	}
}
