#include "pch.hpp"
#include "ColorPickerGUI.hpp"
#include "RaylibUtils.hpp"
#include "Debug.hpp"
#include "ColorPopupGUI.hpp"
#include "PopupGUIManager.hpp"
#include "HelperFunctions.hpp"

ColorPickerGUI::ColorPickerGUI(PopupGUIManager& popupManager, const GUIStyle& settings)
	: SelectableGUI(), m_color(), m_popupManager(&popupManager), m_valueSetCallback() //m_settings(settings), 
{
	//LogWarning(std::format("coloc picker addr create:{}", Utils::ToStringPointerAddress(this)));
	m_OnSelect.AddListener([this](SelectableGUI* gui)-> void
		{
			//Assert(false, "PISS");
			//LogWarning(std::format("color picker call addr:{}", Utils::ToStringPointerAddress(this)));
			ColorPopupGUI* popup = nullptr; 
			const bool isEnabled= m_popupManager->TryTogglePopupAt<ColorPopupGUI>(GetLastFrameRect(), 
				PopupPositionFlags::BelowRect | PopupPositionFlags::CenteredXToRect, &popup);

			if (popup==nullptr)
			{
				Assert(false, std::format("Attempted to popup color popup gui after clicking "
					"color picker GUI:{} but popup failed to appear", ToStringBase()));
			}

			if (isEnabled) popup->SetColor(GetColor());
		});

	/*m_OnDeselect.AddListener([this](SelectableGUI* gui)-> void
		{
			m_popupManager->TryClosePopup<ColorPopupGUI>();
		});*/

	//TODO: maybe make it possible to add events to the popup directly so we do not have to do checks for correct typye
	m_popupManager->m_OnPopupClosed.AddListener([this](const std::string& type, const PopupGUI* popup)-> void
		{
			if (!m_popupManager->IsPopupType<ColorPopupGUI>(type)) return;

			const ColorPopupGUI* colorPopup = dynamic_cast<const ColorPopupGUI*>(popup);
			//Assert(false, std::format("Reached popup of color:{} internal color:{}", colorPopup->GetColor().ToString(), ));
			SetColor(colorPopup->GetColor());
		});
	//Assert(false, std::format(0))
}

void ColorPickerGUI::SetValueSetAction(const ColorPickerAction& action)
{
	m_valueSetCallback = action;
}

void ColorPickerGUI::SetColor(const Utils::Color color) 
{ 
	m_color = color; 
	if (m_valueSetCallback) m_valueSetCallback(m_color);
}
//void ColorPickerGUI::SetSettings(const GUIStyle& settings) { m_settings = settings; }
Utils::Color ColorPickerGUI::GetColor() const { return m_color; }

RenderInfo ColorPickerGUI::ElementRender(const RenderInfo& renderInfo)
{
	//Assert(false, std::format("settings size: {} render info:{}", m_settings.m_Size.ToString(), renderInfo.m_RenderSize.ToString()));

	//Assert(false, std::format("Background color:{}", m_color.ToString()));

	DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y, 
		renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y, RaylibUtils::ToRaylibColor(m_color));
	//if (!IsSelected()) DrawDisabledOverlay({ renderInfo.m_TopLeftPos, {widthUsed, heightUsed} });

	//SetLastFramneRect(GUIRect(renderInfo.m_TopLeftPos, renderInfo.m_RenderSize));

	return renderInfo;
}