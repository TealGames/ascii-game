#include "pch.hpp"
#include "ECS/Systems/Types/UI/UITextSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"

namespace ECS
{
	UITextSystem::UITextSystem() {}
	void UITextSystem::Init()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UITextComponent),
			ComponentInfo(CreateComponentTypes<UIRendererData>(), CreateRequiredComponentFunction(UIRendererData()),
				[](EntityData& entity)-> void
				{
					entity.TryGetComponentMutable<UITextComponent>()->m_renderer = entity.TryGetComponentMutable<UIRendererData>();
				}));
	}
}
