#include "pch.hpp"
#include "ECS/Systems/Types/UI/UIInputFieldSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIPanel.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "Editor/EditorStyles.hpp"
#include "ECS/Systems/MultiBodySystem.hpp" 
#include "Core/Scene/GlobalEntityManager.hpp"

namespace ECS
{
	UIInputFieldSystem::UIInputFieldSystem()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIInputField), ComponentInfo(CreateRequiredComponentFunction<UITextComponent, UIPanel>(
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
