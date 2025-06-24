#include "pch.hpp"
#include "UITextSystem.hpp"
#include "GlobalComponentInfo.hpp"
#include "EntityData.hpp"
#include "UIRendererComponent.hpp"

namespace ECS
{
	UITextSystem::UITextSystem()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UITextComponent), ComponentInfo(CreateRequiredComponentFunction<UIRendererData>(),
			[](EntityData& entity)-> void {entity.TryGetComponentMutable<UITextComponent>()->m_renderer = entity.TryGetComponentMutable<UIRendererData>(); }));
	}
}
