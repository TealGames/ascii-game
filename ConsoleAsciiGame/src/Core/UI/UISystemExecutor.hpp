#pragma once
#include "ECS/Systems/Types/UI/UIButtonSystem.hpp"
#include "ECS/Systems/Types/UI/UITransformSystem.hpp"
#include "ECS/Systems/Types/UI/UISelectableSystem.hpp"
#include "ECS/Systems/Types/Editor/UIColorPickerSystem.hpp"
#include "ECS/Systems/Types/UI/UIInputFieldSystem.hpp"
#include "ECS/Systems/Types/UI/UILayoutSystem.hpp"
#include "ECS/Systems/Types/UI/UIPanelSystem.hpp"
#include "ECS/Systems/Types/UI/UISliderSystem.hpp"
#include "ECS/Systems/Types/UI/UITextSystem.hpp"
#include "ECS/Systems/Types/UI/UIRendererSystem.hpp"
#include "ECS/Systems/Types/UI/UIToggleSystem.hpp"

namespace Rendering { class Renderer; }
class UIHierarhcy;
class GlobalEntityManager;
class EngineState;

class UISystemExecutor
{
private:
	ECS::UIRenderSystem m_uiRenderSystem;

	ECS::UIColorPickerSystem m_colorPickerSystem;
	ECS::UISelectableSystem m_uiSelectableSystem;
	ECS::UITransformSystem m_uiTransformSystem;
	ECS::UIButtonSystem m_uiButtonSystem;
	ECS::UIInputFieldSystem m_uiInputFieldSystem;
	ECS::UILayoutSystem m_uiLayoutSystem;
	ECS::UIPanelSystem m_uiPanelSystem;
	ECS::UISliderSystem m_uiSliderSystem;
	ECS::UITextSystem m_uiTextSystem;
	ECS::UIToggleSystem m_uiToggleSystem;
public:

private:
public:
	UISystemExecutor(const EngineState& state, Rendering::Renderer& renderer, UIHierarchy& hierarchy, PopupUIManager& popupManager);
	void Init();
	void SystemsUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime);
};

