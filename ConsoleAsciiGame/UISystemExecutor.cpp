#include "pch.hpp"
#include "UISystemExecutor.hpp"

UISystemExecutor::UISystemExecutor(Rendering::Renderer& renderer, UIHierarchy& hierarchy, PopupUIManager& popupManager) :
	m_uiRenderSystem(renderer, hierarchy), m_colorPickerSystem(popupManager), m_uiSelectableSystem(), m_uiButtonSystem(), m_uiInputFieldSystem(),
	m_uiLayoutSystem(), m_uiPanelSystem(), m_uiSliderSystem(), m_uiTextSystem(),
	m_uiTransformSystem()
{

}

void UISystemExecutor::SystemsUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime)
{
	m_uiButtonSystem.SystemUpdate(globalEntityManager, deltaTime);
	m_uiInputFieldSystem.SystemUpdate(globalEntityManager, deltaTime);
	m_uiLayoutSystem.SystemUpdate(globalEntityManager, deltaTime);
	m_uiRenderSystem.SystemUpdate();
}
