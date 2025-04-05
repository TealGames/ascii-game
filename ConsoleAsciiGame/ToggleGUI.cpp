#include "pch.hpp"
#include "ToggleGUI.hpp"
#include "RaylibUtils.hpp"
#include "GUISelectorManager.hpp"
#include <optional>

static constexpr int MAX_HEIGHT = 10;

ToggleGUI::ToggleGUI() : 
	SelectableGUI(nullptr), m_isToggled(false), m_settings(), 
	m_valueSetAction(nullptr) {}

ToggleGUI::ToggleGUI(GUISelectorManager& manager, const bool& startValue, const GUISettings& settings, 
	const ToggleAction& valueSetAction)
	: SelectableGUI(&manager),  m_isToggled(startValue), m_settings(settings), 
	m_valueSetAction(valueSetAction)
{
	m_OnClick.AddListener([this](SelectableGUI* self)-> void 
		{
			//LogError("POOP");
			ToggleValue();
			Deselect(); 
		});
}

void ToggleGUI::SetSettings(const GUISettings& settings)
{
	m_settings = settings;
}

const bool& ToggleGUI::IsToggled() const
{
	return m_isToggled;
}

void ToggleGUI::SetValue(const bool& value)
{
	m_isToggled = value;
	if (m_valueSetAction != nullptr) m_valueSetAction(m_isToggled);
}
void ToggleGUI::ToggleValue()
{
	SetValue(!IsToggled());
}

void ToggleGUI::SetValueSetAction(const ToggleAction& action)
{
	m_valueSetAction = action;
}

void ToggleGUI::Update()
{
	if (!IsInit()) Init();
}

ScreenPosition ToggleGUI::Render(const RenderInfo& renderInfo)
{
	const int guiHeight= std::min(renderInfo.m_RenderSize.m_Y, MAX_HEIGHT);
	const int guiWidth = guiHeight;

	Vector2 currentPos= RaylibUtils::ToRaylibVector(renderInfo.m_TopLeftPos);
	currentPos.x += (renderInfo.m_RenderSize.m_X - guiWidth) / 2;

	DrawRectangle(currentPos.x, currentPos.y, guiWidth, guiHeight, m_settings.m_BackgroundColor);
	SetLastFramneRect(GUIRect{ ScreenPosition(currentPos.x, currentPos.y), {guiWidth, guiHeight} });
	if (IsToggled())
	{
		const int radius = std::min(guiWidth, guiHeight) / 2;
		DrawCircle(currentPos.x+radius, currentPos.y+radius, radius, WHITE);
	}

	//Since we do not really have different states between focus input and settings (it is done in one go)
	//we can always draw the disabled overlay
	DrawDisabledOverlay({ ScreenPosition{static_cast<int>(currentPos.x),
				static_cast<int>(currentPos.y) }, ScreenPosition{guiWidth, guiHeight} });

	GetLastFrameRectMutable().SetTopLeftPos(renderInfo.m_TopLeftPos);
	GetLastFrameRectMutable().SetSize({renderInfo.m_RenderSize.m_X, guiHeight });
	return GetLastFrameRect().GetSize();
}