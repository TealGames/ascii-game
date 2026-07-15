#include "pch.hpp"
#include "ECS/Systems/Types/UI/UISliderSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/UI/UISliderComponent.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"
#include "ECS/Component/Types/UI/UISelectableComponent.hpp"

namespace Engine::UI
{
	UISliderSystem::UISliderSystem(const Input::InputManager& inputManager) : m_inputManager(&inputManager) {}
	void UISliderSystem::Init()
	{
		ECS::GlobalComponentInfo::AddComponentInfo(typeid(UISliderComponent),
			ECS::ComponentInfo(ECS::CreateComponentTypeInfo<UIRendererComponent, UIPanelComponent, UISelectableComponent>(),
				CreateRequiredComponentFunction(UIRendererComponent(), UIPanelComponent(), UISelectableComponent()),
				[this](ECS::EntityData& entity)-> void
				{
					UISliderComponent& slider = *(entity.TryGetComponentMutable<UISliderComponent>());
					slider.m_renderer = entity.TryGetComponentMutable<UIRendererComponent>();
					slider.m_backgroundPanel = entity.TryGetComponentMutable<UIPanelComponent>();
					slider.m_selectable = entity.TryGetComponentMutable<UISelectableComponent>();

					slider.Init(*m_inputManager);
				}));
	}
}
