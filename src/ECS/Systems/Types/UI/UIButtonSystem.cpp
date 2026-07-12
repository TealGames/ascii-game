#include "pch.hpp"
#include "ECS/Systems/Types/UI/UIButtonSystem.hpp"
#include "ECS/Systems/MultiBodySystem.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "ECS/Component/Types/UI/UIButtonComponent.hpp"	
#include "ECS/Component/Types/UI/UISelectableComponent.hpp"
#include "Editor/EditorStyles.hpp"

namespace Engine::UI
{
	UIButtonSystem::UIButtonSystem() {}
	void UIButtonSystem::Init()
	{
		//TODO: decide -> should button allow required components from other entities?? if so technically we should not require
		//text and selectable since it will look for on its own entity and instead maybe it is a different variant of required where
		//it checks in the post add action if it has those component refs after construction and if not force adds them?
		ECS::GlobalComponentInfo::AddComponentInfo(typeid(UIButtonComponent),
			ECS::ComponentInfo(ECS::CreateComponentTypes<UITextComponent, UISelectableComponent>(),
				ECS::CreateRequiredComponentFunction(UITextComponent("", Editor::Styles::GetTextStyleFactorSize(TextAlignment::Center)), UISelectableComponent()),
				[](ECS::EntityData& entity) -> void
				{
					UIButtonComponent& button = *(entity.TryGetComponentMutable<UIButtonComponent>());
					//NOTE: since button allows text and selectable component in constructor AND they are not checked to be part 
					//of the same button entity, it means we do not want to override with its components if it has other references it wants to use
					if (button.m_textGUI != nullptr) button.m_textGUI = entity.TryGetComponentMutable<UITextComponent>();
					if (button.m_selectable != nullptr) button.m_selectable = entity.TryGetComponentMutable<UISelectableComponent>();
				}));
	}

	void UIButtonSystem::SystemUpdate(Scenes::GlobalEntityManager& globalEntityManager, const float& deltaTime)
	{
		globalEntityManager.OperateOnComponents<UIButtonComponent>(ECS::ALL_ACTIVE_ENABLED_FLAG,
			[this, &deltaTime](UIButtonComponent& data)-> void
			{
				data.Update(deltaTime);
			});
	}
}

