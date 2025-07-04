#include "pch.hpp"
#include "ECS/Component/Types/Editor/UIColorPicker.hpp"
#include "Utils/RaylibUtils.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Editor/Entity/ColorPopupUI.hpp"
#include "Core/UI/PopupUIManager.hpp"
#include "Utils/HelperFunctions.hpp"
#include "ECS/Component/Types/UI/UIPanel.hpp"
#include "ECS/Component/Types/UI/UISelectableData.hpp"

UIColorPickerData::UIColorPickerData() : UIColorPickerData(UIStyle()) {}
UIColorPickerData::UIColorPickerData(const UIStyle& settings)
	: m_color(), m_popupManager(nullptr), m_valueSetCallback(), m_renderer(nullptr), m_fieldPanel(nullptr), m_selectable(nullptr)
{
	//LogWarning(std::format("coloc picker addr create:{}", Utils::ToStringPointerAddress(this)));
	

	/*m_OnDeselect.AddListener([this](SelectableGUI* gui)-> void
		{
			m_popupManager->TryClosePopup<ColorPopupGUI>();
		});*/

	//TODO: maybe make it possible to add events to the popup directly so we do not have to do checks for correct typye
	m_popupManager->m_OnPopupClosed.AddListener([this](const std::string& type, const PopupUI* popup)-> void
		{
			if (!m_popupManager->IsPopupType<ColorPopupUI>(type)) return;

			const ColorPopupUI* colorPopup = dynamic_cast<const ColorPopupUI*>(popup);
			//Assert(false, std::format("Reached popup of color:{} internal color:{}", colorPopup->GetColor().ToString(), ));
			SetColor(colorPopup->GetColor());
		});
	//Assert(false, std::format(0))
}

void UIColorPickerData::Init()
{
	m_selectable->m_OnSelect.AddListener([this](UISelectableData* gui)-> void
		{
			//Assert(false, "PISS");
			//LogWarning(std::format("color picker call addr:{}", Utils::ToStringPointerAddress(this)));
			ColorPopupUI* popup = nullptr;
			const bool isEnabled = m_popupManager->TryTogglePopupAt<ColorPopupUI>(GetEntity().TryGetComponent<UITransformData>()->GetLastWorldArea(),
				PopupPositionFlags::BelowRect | PopupPositionFlags::CenteredXToRect, &popup);

			if (popup == nullptr)
			{
				Assert(false, std::format("Attempted to popup color popup gui after clicking "
					"color picker GUI:{} but popup failed to appear", ToString()));
			}

			if (isEnabled) popup->SetColor(GetColor());
		});
}

void UIColorPickerData::SetValueSetAction(const ColorPickerAction& action)
{
	m_valueSetCallback = action;
}

void UIColorPickerData::SetColor(const Utils::Color color) 
{ 
	m_color = color; 
	if (m_fieldPanel != nullptr) m_fieldPanel->SetColor(RaylibUtils::ToRaylibColor(m_color));
	if (m_valueSetCallback) m_valueSetCallback(m_color);
}
//void ColorPickerGUI::SetSettings(const GUIStyle& settings) { m_settings = settings; }
Utils::Color UIColorPickerData::GetColor() const { return m_color; }
const UIPanel* UIColorPickerData::GetFieldPanel() const { return m_fieldPanel; }

void UIColorPickerData::InitFields()
{
	m_Fields = {};
}
std::string UIColorPickerData::ToString() const
{
	return std::format("[UIColorPicker]");
}

void UIColorPickerData::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json UIColorPickerData::Serialize()
{
	//TODO: implement
	return {};
}

/*
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
*/