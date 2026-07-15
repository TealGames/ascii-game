#include "pch.hpp"
#include "ECS/Systems/Types/UI/UIPanelSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

namespace Engine::UI
{
	UIPanelSystem::UIPanelSystem() {}

	void UIPanelSystem::Init()
	{
		ECS::GlobalComponentInfo::AddComponentInfo(typeid(UIPanelComponent),
			ECS::ComponentInfo(ECS::CreateComponentTypeInfo<UIRendererComponent>(), CreateRequiredComponentFunction(UIRendererComponent()),
				[](ECS::EntityData& entity)-> void
				{
					entity.TryGetComponentMutable<UIPanelComponent>()->m_renderer = entity.TryGetComponentMutable<UIRendererComponent>();
				}));
	}
}
