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

namespace Engine::Rendering { class Renderer; }
namespace Engine::UI { class UIHierarhcy; }
namespace Engine::Scenes { class GlobalEntityManager; }
namespace Engine::Core { class EngineState; }
namespace Engine::UI
{
	class UISystemExecutor
	{
	private:
	public:
		//TODO: is it really a good idea that an editor UI is grouped with other UI types??
		Editor::UI::UIColorPickerSystem m_ColorPickerSystem;

		UIRenderSystem m_UiRenderSystem;
		UISelectableSystem m_UiSelectableSystem;
		UITransformSystem m_UiTransformSystem;
		UIButtonSystem m_UiButtonSystem;
		UIInputFieldSystem m_UiInputFieldSystem;
		UILayoutSystem m_UiLayoutSystem;
		UIPanelSystem m_UiPanelSystem;
		UISliderSystem m_UiSliderSystem;
		UITextSystem m_UiTextSystem;
		UITextureSystem m_UiTextureSystem;
		UIToggleSystem m_UiToggleSystem;

	private:
	public:
		UISystemExecutor(const Core::EngineState& state, Rendering::Renderer& renderer, UIHierarchy& hierarchy, PopupUIManager& popupManager);
		void Init();
		void SystemsUpdate(Scenes::GlobalEntityManager& globalEntityManager, const float& deltaTime);
	};
}


