#include "pch.hpp"
#include "ECS/Systems/Types/UI/UIButtonSystem.hpp"
#include "ECS/Systems/MultiBodySystem.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "Editor/EditorStyles.hpp"

namespace ECS
{
	UIButtonSystem::UIButtonSystem() 
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIButton), ComponentInfo(CreateRequiredComponentFunction<UITextComponent>(
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

