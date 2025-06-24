#pragma once
#include "UIButtonSystem.hpp"
#include "UITransformSystem.hpp"
#include "UISelectableSystem.hpp"
#include "UIColorPickerSystem.hpp"
#include "UIInputFieldSystem.hpp"
#include "UILayoutSystem.hpp"
#include "UIPanelSystem.hpp"
#include "UISliderSystem.hpp"
#include "UITextSystem.hpp"
#include "UIRendererSystem.hpp"

namespace Rendering { class Renderer; }
class UIHierarhcy;
class GlobalEntityManager;
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
public:

private:
public:
	UISystemExecutor(Rendering::Renderer& renderer, UIHierarchy& hierarchy, PopupUIManager& popupManager);
	void SystemsUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime);
};

