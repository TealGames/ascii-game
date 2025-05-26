#include "pch.hpp"
#include "ToggleGUI.hpp"
#include "RaylibUtils.hpp"
#include "GUISelectorManager.hpp"
#include <optional>
#include "TextureAsset.hpp"

//ToggleGUI::ToggleGUI() : 
//	SelectableGUI(nullptr), m_isToggled(false), m_settings(), 
//	m_valueSetAction(nullptr) {}

ToggleGUI::ToggleGUI(const bool& startValue, const GUIStyle& settings, 
	const ToggleAction& valueSetAction, const TextureAsset* toggledTexture)
	: SelectableGUI(),  m_isToggled(startValue), m_settings(settings), 
	m_valueSetAction(valueSetAction), m_overlayTexture(toggledTexture)
{
	m_OnClick.AddListener([this](SelectableGUI* self)-> void 
		{
			//Assert(false, "POOP");
			ToggleValue();
			self->Deselect();
			//SetSettings({});
		});
}

ToggleGUI::~ToggleGUI()
{
	//LogError(std::format("Toggle destroyed"));
	/*if (m_settings.m_Size == ScreenPosition{69, 69}) 
		Assert(false, std::format("DEATRUCTOR HELL YEAH settings:{}", m_settings.m_Size.ToString()));*/
}

void ToggleGUI::SetSettings(const GUIStyle& settings)
{
	m_settings = settings;
}
void ToggleGUI::SetOverlayTexture(const TextureAsset& asset)
{
	m_overlayTexture = &asset;
}
bool ToggleGUI::HasOverlayTexture() const
{
	return m_overlayTexture != nullptr;
}

bool ToggleGUI::IsToggled() const
{
	return m_isToggled;
}

void ToggleGUI::SetValue(const bool value)
{
	m_isToggled = value;
	if (m_valueSetAction) m_valueSetAction(m_isToggled);
}
void ToggleGUI::ToggleValue()
{
	SetValue(!m_isToggled);
}

void ToggleGUI::SetValueSetAction(const ToggleAction& action)
{
	m_valueSetAction = action;
}

void ToggleGUI::DrawOverlayTexture(const float targetWidth, const float targetHeight, const Vector2& topLeftPos)
{
	const float scaleX = targetWidth / m_overlayTexture->GetTexture().width;
	const float scaleY = targetHeight / m_overlayTexture->GetTexture().height;
	DrawTextureEx(m_overlayTexture->GetTexture(), topLeftPos, 0, std::min(scaleX, scaleY), WHITE);
}

RenderInfo ToggleGUI::ElementRender(const RenderInfo& renderInfo)
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