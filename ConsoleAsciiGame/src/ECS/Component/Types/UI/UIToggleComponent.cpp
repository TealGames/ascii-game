#include "pch.hpp"
#include "ECS/Component/Types/UI/UIToggleComponent.hpp"
#include "Utils/RaylibUtils.hpp"
#include "Core/UI/UIInteractionManager.hpp"
#include <optional>
#include "Core/Asset/TextureAsset.hpp"
#include "ECS/Component/Types/UI/UISelectableData.hpp"
#include "ECS/Component/Types/UI/UITextureData.hpp"

//ToggleGUI::ToggleGUI() : 
//	SelectableGUI(nullptr), m_isToggled(false), m_settings(), 
//	m_valueSetAction(nullptr) {}

UIToggleComponent::UIToggleComponent(const bool& startValue, const UIStyle& settings, UITextureData* onTexture, UITextureData* offTexture)//, const TextureAsset* toggledTexture)
	: m_isToggled(startValue), m_settings(settings), 
	//m_valueSetAction(valueSetAction), 
	m_OnValueSet(), m_onTexture(onTexture), m_offTexture(offTexture),
	m_selectable(nullptr)//,m_overlayTexture(toggledTexture),
{
	SetTextureFromState();
}

UIToggleComponent::~UIToggleComponent()
{
	//LogError(std::format("Toggle destroyed"));
	/*if (m_settings.m_Size == ScreenPosition{69, 69}) 
		Assert(false, std::format("DEATRUCTOR HELL YEAH settings:{}", m_settings.m_Size.ToString()));*/
}

void UIToggleComponent::Init()
{
	m_selectable->m_OnClick.AddListener([this](UISelectableData* self)-> void
		{
			//Assert(false, "POOP");
			ToggleValue();
			self->Deselect();
			//SetSettings({});
		});
}

void UIToggleComponent::SetSettings(const UIStyle& settings)
{
	m_settings = settings;
}
void UIToggleComponent::SetStateTextures(UITextureData* onTexture, UITextureData* offTexture)
{
	m_onTexture = onTexture;
	m_offTexture = offTexture;
}
void UIToggleComponent::SetTextureFromState()
{
	if (m_onTexture != nullptr) m_onTexture->m_IsEnabled = m_isToggled;
	if (m_offTexture != nullptr) m_offTexture->m_IsEnabled = !m_isToggled;
}
//void UIToggleComponent::SetOverlayTexture(const TextureAsset& asset)
//{
//	m_overlayTexture = &asset;
//}
//bool UIToggleComponent::HasOverlayTexture() const
//{
//	return m_overlayTexture != nullptr;
//}

bool UIToggleComponent::IsToggled() const
{
	return m_isToggled;
}

void UIToggleComponent::SetValue(const bool value)
{
	m_isToggled = value;
	SetTextureFromState();
	//if (m_valueSetAction) m_valueSetAction(m_isToggled);
	m_OnValueSet.Invoke(m_isToggled);
}
void UIToggleComponent::ToggleValue()
{
	SetValue(!m_isToggled);
}

//void UIToggleComponent::SetValueSetAction(const ToggleAction& action)
//{
//	m_valueSetAction = action;
//}

//void UIToggleComponent::DrawOverlayTexture(const float targetWidth, const float targetHeight, const Vector2& topLeftPos)
//{
//	const float scaleX = targetWidth / m_overlayTexture->GetTexture().width;
//	const float scaleY = targetHeight / m_overlayTexture->GetTexture().height;
//	DrawTextureEx(m_overlayTexture->GetTexture(), topLeftPos, 0, std::min(scaleX, scaleY), WHITE);
//}

/*
RenderInfo UIToggleComponent::ElementRender(const RenderInfo& renderInfo)
{
	const float guiHeight = renderInfo.m_RenderSize.m_Y;
	const float guiWidth = guiHeight;

	Vector2 topLeftPos= RaylibUtils::ToRaylibVector(renderInfo.m_TopLeftPos);
	topLeftPos.x += (renderInfo.m_RenderSize.m_X - guiWidth) / 2;

	DrawRectangle(topLeftPos.x, topLeftPos.y, guiWidth, guiHeight, m_settings.m_BackgroundColor);
	if (IsToggled())
	{
		if (HasOverlayTexture()) DrawOverlayTexture(guiWidth, guiHeight, topLeftPos);
		else
		{
			const int radius = std::min(guiWidth, guiHeight) / 2;
			DrawCircle(topLeftPos.x + radius, topLeftPos.y + radius, radius, m_settings.m_SecondaryColor);
		}
	}
	else if (HasOverlayTexture()) DrawOverlayTexture(guiWidth, guiHeight, topLeftPos);

	//Since we do not really have different states between focus input and settings (it is done in one go)
	//we can always draw the disabled overlay
	//DrawDisabledOverlay(overlayRenderInfo);
	//if (IsHoveredOver()) DrawHoverOverlay(overlayRenderInfo);

	//SetLastFramneRect(GUIRect{ ScreenPosition(topLeftPos.x, topLeftPos.y), {guiWidth, guiHeight} });
	return RenderInfo{ ScreenPosition{static_cast<int>(topLeftPos.x),
				static_cast<int>(topLeftPos.y) }, ScreenPosition(guiWidth, guiHeight) };
}
*/

void UIToggleComponent::InitFields()
{
	m_Fields = {};
}
std::string UIToggleComponent::ToString() const
{
	return std::format("[ToggleGUI]");
}

void UIToggleComponent::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json UIToggleComponent::Serialize()
{
	//TODO: implement
	return {};
}