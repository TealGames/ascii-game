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
#include "ECS/Systems/Types/UI/UITextureSystem.hpp"

namespace Rendering { class Renderer; }
class UIHierarhcy;
class GlobalEntityManager;
class EngineState;

class UISystemExecutor
{
private:
public:
	ECS::UIRenderSystem m_UiRenderSystem;

	ECS::UIColorPickerSystem m_ColorPickerSystem;
	ECS::UISelectableSystem m_UiSelectableSystem;
	ECS::UITransformSystem m_UiTransformSystem;
	ECS::UIButtonSystem m_UiButtonSystem;
	ECS::UIInputFieldSystem m_UiInputFieldSystem;
	ECS::UILayoutSystem m_UiLayoutSystem;
	ECS::UIPanelSystem m_UiPanelSystem;
	ECS::UISliderSystem m_UiSliderSystem;
	ECS::UITextSystem m_UiTextSystem;
	ECS::UITextureSystem m_UiTextureSystem;
	ECS::UIToggleSystem m_UiToggleSystem;

private:
public:
	UISystemExecutor(const EngineState& state, Rendering::Renderer& renderer, UIHierarchy& hierarchy, PopupUIManager& popupManager);
	void Init();
	void SystemsUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime);
};

