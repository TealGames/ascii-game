#include "pch.hpp"
#include "ECS/Systems/Types/UI/UIInputFieldSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityComponent.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "Editor/EditorStyles.hpp"
#include "ECS/Systems/MultiBodySystem.hpp" 
#include "Core/Scene/GlobalEntityManager.hpp"
#include "ECS/Component/Types/UI/UISelectableData.hpp"

namespace ECS
{
	UIInputFieldSystem::UIInputFieldSystem() {}
	void UIInputFieldSystem::Init()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIInputFieldComponent),
			ComponentInfo(CreateComponentTypes<UITextComponent, UIPanelComponent, UISelectableData>(),
				CreateRequiredComponentFunction<UITextComponent, UIPanelComponent, UISelectableData>(
					UITextComponent("", EditorStyles::GetTextStyleFactorSize(TextAlignment::Center)), UIPanelComponent(), UISelectableData()),
				[](EntityData& entity)-> void
				{
					UIInputFieldComponent& fieldComponent = *(entity.TryGetComponentMutable<UIInputFieldComponent>());
					fieldComponent.m_textGUI = entity.TryGetComponentMutable<UITextComponent>();
					fieldComponent.m_background = entity.TryGetComponentMutable<UIPanelComponent>();
					fieldComponent.m_selectable = entity.TryGetComponentMutable<UISelectableData>();

					fieldComponent.Init();
				}));
	}

	void UIInputFieldSystem::SystemUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime)
	{
		globalEntityManager.OperateOnComponents<UIInputFieldComponent>(ALL_ACTIVE_ENABLED_FLAG,
			[this](UIInputFieldComponent& data)-> void
			{
				data.Update();
			});
	}
}
