#include "pch.hpp"
#include "UITextureSystem.hpp"
#include "GlobalComponentInfo.hpp"
#include "EntityData.hpp"
#include "UIRendererComponent.hpp"

namespace ECS
{
	UITextureSystem::UITextureSystem()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UITextureData), ComponentInfo(RequiredComponentCheck<UIRendererData>(),
			[](EntityData& entity)-> void {entity.TryGetComponentMutable<UITextureData>()->m_renderer = entity.TryGetComponentMutable<UIRendererData>(); }));
	}
}
