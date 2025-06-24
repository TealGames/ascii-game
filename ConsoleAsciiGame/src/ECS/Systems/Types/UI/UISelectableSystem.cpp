#include "pch.hpp"
#include "ECS/Systems/Types/UI/UISelectableSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

namespace ECS
{
	UISelectableSystem::UISelectableSystem()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UISelectableData), ComponentInfo(CreateRequiredComponentFunction<UIRendererData>(),
			[](EntityData& entity)-> void {entity.TryGetComponentMutable<UISelectableData>()->m_renderer = entity.TryGetComponentMutable<UIRendererData>(); }));
	}
}
