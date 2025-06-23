#include "pch.hpp"
#include "UIInputFieldSystem.hpp"
#include "GlobalComponentInfo.hpp"
#include "EntityData.hpp"
#include "UIPanel.hpp"
#include "UITextComponent.hpp"
#include "EditorStyles.hpp"
#include "MultiBodySystem.hpp" 
#include "GlobalEntityManager.hpp"

namespace ECS
{
	UIInputFieldSystem::UIInputFieldSystem()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIInputField), ComponentInfo(RequiredComponentCheck<UITextComponent, UIPanel>(
			UITextComponent("", EditorStyles::GetTextStyleFactorSize(TextAlignment::Center)), UIPanel()),
			[](EntityData& entity)-> void 
			{
				UIInputField& fieldComponent = *(entity.TryGetComponentMutable<UIInputField>());
				fieldComponent.m_textGUI = entity.TryGetComponentMutable<UITextComponent>();
				fieldComponent.m_background = entity.TryGetComponentMutable<UIPanel>();
			}));
	}

	void UIInputFieldSystem::SystemUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime)
	{
		globalEntityManager.OperateOnComponents<UIInputField>(
			[this, &deltaTime](UIInputField& data)-> void
			{
				data.Update(deltaTime);
			});
	}
}
