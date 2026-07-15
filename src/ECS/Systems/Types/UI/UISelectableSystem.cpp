#include "pch.hpp"
#include "ECS/Systems/Types/UI/UISelectableSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/UI/UISelectableComponent.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

namespace Engine::UI
{
	UISelectableSystem::UISelectableSystem() {}
	void UISelectableSystem::Init()
	{
		ECS::GlobalComponentInfo::AddComponentInfo(typeid(UISelectableComponent),
			ECS::ComponentInfo(ECS::CreateComponentTypeInfo<UIRendererComponent>(), CreateRequiredComponentFunction(UIRendererComponent()),
				[](ECS::EntityData& entity)-> void
				{
					entity.TryGetComponentMutable<UISelectableComponent>()->m_renderer = entity.TryGetComponentMutable<UIRendererComponent>();
				}));
	}
}
