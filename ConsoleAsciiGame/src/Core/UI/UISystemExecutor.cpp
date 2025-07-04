#include "pch.hpp"
#include "Core/UI/UISystemExecutor.hpp"

UISystemExecutor::UISystemExecutor(const EngineState& state, Rendering::Renderer& renderer, UIHierarchy& hierarchy, PopupUIManager& popupManager) :
	m_uiRenderSystem(state, renderer, hierarchy), m_colorPickerSystem(popupManager), m_uiSelectableSystem(), m_uiButtonSystem(), m_uiInputFieldSystem(),
	m_uiLayoutSystem(), m_uiPanelSystem(), m_uiSliderSystem(), m_uiTextSystem(), m_uiToggleSystem(), m_uiTransformSystem()
{

}
void UISystemExecutor::Init()
{
	m_uiRenderSystem.Init();
	m_colorPickerSystem.Init();
	m_uiSelectableSystem.Init();
	m_uiButtonSystem.Init();
	m_uiInputFieldSystem.Init();
	m_uiPanelSystem.Init();
	m_uiSliderSystem.Init();
	m_uiTextSystem.Init();
	m_uiToggleSystem.Init();
}

void UISystemExecutor::SystemsUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime)
{
	m_uiButtonSystem.SystemUpdate(globalEntityManager, deltaTime);
	m_uiInputFieldSystem.SystemUpdate(globalEntityManager, deltaTime);
	m_uiLayoutSystem.SystemUpdate(globalEntityManager, deltaTime);
	m_uiRenderSystem.SystemUpdate();
}
