#include "pch.hpp"
#include "ECS/Systems/Types/UI/UITextSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"

namespace Engine::UI
{
	UITextSystem::UITextSystem() {}
	void UITextSystem::Init()
	{
		ECS::GlobalComponentInfo::AddComponentInfo(typeid(UITextComponent),
			ECS::ComponentInfo(ECS::CreateComponentTypeInfo<UIRendererComponent>(), CreateRequiredComponentFunction(UIRendererComponent()),
				[](ECS::EntityData& entity)-> void
				{
					entity.TryGetComponentMutable<UITextComponent>()->m_renderer = entity.TryGetComponentMutable<UIRendererComponent>();
				}));
	}
}
