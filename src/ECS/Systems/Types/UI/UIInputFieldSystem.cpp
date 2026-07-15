#include "pch.hpp"
#include "ECS/Systems/Types/UI/UIInputFieldSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "Editor/EditorStyles.hpp"
#include "ECS/Systems/MultiBodySystem.hpp" 
#include "Core/Scene/GlobalEntityManager.hpp"
#include "ECS/Component/Types/UI/UISelectableComponent.hpp"
#include "ECS/Component/Types/UI/UIInputFieldComponent.hpp"

namespace Engine::UI
{
	UIInputFieldSystem::UIInputFieldSystem(const Input::InputManager& input) : m_inputManager(&input){}
	void UIInputFieldSystem::Init()
	{
		ECS::GlobalComponentInfo::AddComponentInfo(typeid(UIInputFieldComponent),
			ECS::ComponentInfo(ECS::CreateComponentTypeInfo<UITextComponent, UIPanelComponent, UISelectableComponent>(),
				CreateRequiredComponentFunction<UITextComponent, UIPanelComponent, UISelectableComponent>(
					UITextComponent("", Editor::Styles::GetTextStyleFactorSize(TextAlignment::Center)), UIPanelComponent(), UISelectableComponent()),
				[this](ECS::EntityData& entity)-> void
				{
					UIInputFieldComponent& fieldComponent = *(entity.TryGetComponentMutable<UIInputFieldComponent>());
					fieldComponent.m_textGUI = entity.TryGetComponentMutable<UITextComponent>();
					fieldComponent.m_background = entity.TryGetComponentMutable<UIPanelComponent>();
					fieldComponent.m_selectable = entity.TryGetComponentMutable<UISelectableComponent>();

					fieldComponent.Init(*m_inputManager);
				}));
	}

	void UIInputFieldSystem::SystemUpdate(Scenes::GlobalEntityManager& globalEntityManager, const float& deltaTime)
	{
		globalEntityManager.OperateOnComponents<UIInputFieldComponent>(ECS::ALL_ACTIVE_ENABLED_FLAG,
			[this](UIInputFieldComponent& data)-> void
			{
				data.Update();
			});
	}
}
