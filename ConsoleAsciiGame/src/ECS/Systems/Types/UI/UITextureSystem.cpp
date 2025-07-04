#include "pch.hpp"
#include "ECS/Systems/Types/UI/UITextureSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"

namespace ECS
{
	UITextureSystem::UITextureSystem() {}
	void UITextureSystem::Init()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UITextureData),
			ComponentInfo(CreateComponentTypes<UIRendererData>(), CreateRequiredComponentFunction(UIRendererData()),
				[](EntityData& entity)-> void
				{
					entity.TryGetComponentMutable<UITextureData>()->m_renderer = entity.TryGetComponentMutable<UIRendererData>();
				}));
	}
}
