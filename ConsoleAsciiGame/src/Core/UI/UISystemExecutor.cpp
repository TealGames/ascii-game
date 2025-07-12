#include "pch.hpp"
#include "Core/UI/UISystemExecutor.hpp"

UISystemExecutor::UISystemExecutor(const EngineState& state, Rendering::Renderer& renderer, UIHierarchy& hierarchy, PopupUIManager& popupManager) :
	m_UiRenderSystem(state, renderer, hierarchy), m_ColorPickerSystem(popupManager), m_UiSelectableSystem(), m_UiButtonSystem(), m_UiInputFieldSystem(),
	m_UiLayoutSystem(), m_UiPanelSystem(), m_UiSliderSystem(), m_UiTextSystem(), m_UiToggleSystem(), m_UiTransformSystem()
{

}
void UISystemExecutor::Init()
{
	m_UiRenderSystem.Init();
	m_ColorPickerSystem.Init();
	m_UiSelectableSystem.Init();
	m_UiButtonSystem.Init();
	m_UiInputFieldSystem.Init();
	m_UiPanelSystem.Init();
	m_UiSliderSystem.Init();
	m_UiTextSystem.Init();
	m_UiToggleSystem.Init();
	m_UiTextureSystem.Init();
}

void UISystemExecutor::SystemsUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime)
{
	m_UiButtonSystem.SystemUpdate(globalEntityManager, deltaTime);
	m_UiInputFieldSystem.SystemUpdate(globalEntityManager, deltaTime);
	m_UiLayoutSystem.SystemUpdate(globalEntityManager, deltaTime);
	m_UiRenderSystem.SystemUpdate();
}
