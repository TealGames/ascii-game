#include "pch.hpp"
#include "ECS/Systems/Types/UI/UIButtonSystem.hpp"
#include "ECS/Systems/MultiBodySystem.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "ECS/Component/Types/UI/UISelectableData.hpp"
#include "Editor/EditorStyles.hpp"

namespace ECS
{
	UIButtonSystem::UIButtonSystem() {}
	void UIButtonSystem::Init()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIButtonComponent),
			ComponentInfo(CreateComponentTypes<UITextComponent, UISelectableData>(),
				CreateRequiredComponentFunction(UITextComponent("", EditorStyles::GetTextStyleFactorSize(TextAlignment::Center)), UISelectableData()),
				[](EntityData& entity)-> void
				{
					UIButtonComponent& button = *(entity.TryGetComponentMutable<UIButtonComponent>());
					button.m_textGUI = entity.TryGetComponentMutable<UITextComponent>();
					button.m_selectable = entity.TryGetComponentMutable<UISelectableData>();
				}));
	}

	void UIButtonSystem::SystemUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime)
	{
		globalEntityManager.OperateOnComponents<UIButtonComponent>(ALL_ACTIVE_ENABLED_FLAG,
			[this, &deltaTime](UIButtonComponent& data)-> void
			{
				data.Update(deltaTime);
			});
	}
}

