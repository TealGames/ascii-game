#include "pch.hpp"
#include "UIButtonSystem.hpp"
#include "MultiBodySystem.hpp"
#include "GlobalEntityManager.hpp"
#include "GlobalComponentInfo.hpp"
#include "UITextComponent.hpp"
#include "EditorStyles.hpp"

namespace ECS
{
	UIButtonSystem::UIButtonSystem() 
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIButton), ComponentInfo(RequiredComponentCheck<UITextComponent>(
			UITextComponent("", EditorStyles::GetTextStyleFactorSize(TextAlignment::Center))), 
			[](EntityData& entity)-> void {entity.TryGetComponentMutable<UIButton>()->m_textGUI = entity.TryGetComponentMutable<UITextComponent>(); }));
	}

	void UIButtonSystem::SystemUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime)
	{
		globalEntityManager.OperateOnComponents<UIButton>(
			[this, &deltaTime](UIButton& data)-> void
			{
				data.Update(deltaTime);
			});
	}
}

