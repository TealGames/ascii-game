#include "pch.hpp"
#include "ColorPickerGUI.hpp"
#include "RaylibUtils.hpp"
#include "Debug.hpp"
#include "ColorPopupGUI.hpp"
#include "PopupGUIManager.hpp"
#include "HelperFunctions.hpp"

ColorPickerGUI::ColorPickerGUI(PopupGUIManager& popupManager, const GUIStyle& settings)
	: SelectableGUI(), m_settings(settings), m_color(), m_popupManager(&popupManager)
{
	//LogWarning(std::format("coloc picker addr create:{}", Utils::ToStringPointerAddress(this)));
	m_OnSelect.AddListener([this](SelectableGUI* gui)-> void
		{
			//Assert(false, "PISS");
			//LogWarning(std::format("color picker call addr:{}", Utils::ToStringPointerAddress(this)));
			m_popupManager->TryTogglePopupBelowRect<ColorPopupGUI>(GetLastFrameRectMutable());
		});
	//Assert(false, std::format(0))
}

void ColorPickerGUI::SetColor(const Utils::Color color) { m_color = color; }
void ColorPickerGUI::SetSettings(const GUIStyle& settings) { m_settings = settings; }
Utils::Color ColorPickerGUI::GetColor() const { return m_color; }

RenderInfo ColorPickerGUI::Render(const RenderInfo& renderInfo)
{
	//Assert(false, std::format("settings size: {} render info:{}", m_settings.m_Size.ToString(), renderInfo.m_RenderSize.ToString()));

	//Assert(false, std::format("Background color:{}", m_color.ToString()));

	DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y, 
		renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y, RaylibUtils::ToRaylibColor(m_color));
	//if (!IsSelected()) DrawDisabledOverlay({ renderInfo.m_TopLeftPos, {widthUsed, heightUsed} });

	SetLastFramneRect(GUIRect(renderInfo.m_TopLeftPos, renderInfo.m_RenderSize));

	return renderInfo;
}