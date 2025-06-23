#include "pch.hpp"
#include "UISelectableSystem.hpp"
#include "GlobalComponentInfo.hpp"
#include "UIRendererComponent.hpp"
#include "EntityData.hpp"

namespace ECS
{
	UISelectableSystem::UISelectableSystem()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UISelectableData), ComponentInfo(RequiredComponentCheck<UIRendererData>(),
			[](EntityData& entity)-> void {entity.TryGetComponentMutable<UISelectableData>()->m_renderer = entity.TryGetComponentMutable<UIRendererData>(); }));
	}
}
