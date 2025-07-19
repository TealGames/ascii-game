#include "pch.hpp"
#include "ECS/Systems/Types/UI/UIInputFieldSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIPanel.hpp"
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
		GlobalComponentInfo::AddComponentInfo(typeid(UIInputField),
			ComponentInfo(CreateComponentTypes<UITextComponent, UIPanel, UISelectableData>(),
				CreateRequiredComponentFunction<UITextComponent, UIPanel, UISelectableData>(
					UITextComponent("", EditorStyles::GetTextStyleFactorSize(TextAlignment::Center)), UIPanel(), UISelectableData()),
				[](EntityData& entity)-> void
				{
					UIInputField& fieldComponent = *(entity.TryGetComponentMutable<UIInputField>());
					fieldComponent.m_textGUI = entity.TryGetComponentMutable<UITextComponent>();
					fieldComponent.m_background = entity.TryGetComponentMutable<UIPanel>();
					fieldComponent.m_selectable = entity.TryGetComponentMutable<UISelectableData>();

					fieldComponent.Init();
				}));
	}

	void UIInputFieldSystem::SystemUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime)
	{
		globalEntityManager.OperateOnComponents<UIInputField>(
			[this](UIInputField& data)-> void
			{
				data.Update();
			});
	}
}
