#include "pch.hpp"
#include "CheckboxGUI.hpp"
#include "RaylibUtils.hpp"
#include "GUISelectorManager.hpp"
#include <optional>

static constexpr int MAX_HEIGHT = 10;

CheckboxGUI::CheckboxGUI() : 
	SelectableGUI(nullptr), m_value(false), m_settings(), 
	m_lastFrameCheckboxRect(), m_valueSetAction(nullptr) {}

CheckboxGUI::CheckboxGUI(GUISelectorManager& manager, const bool& startValue, const GUISettings& settings, 
	const CheckboxAction& valueSetAction)
	: SelectableGUI(&manager),  m_value(startValue), m_settings(settings), 
	m_lastFrameCheckboxRect(), m_valueSetAction(valueSetAction){}

const bool& CheckboxGUI::IsChecked() const
{
	return m_value;
}

void CheckboxGUI::SetValue(const bool& value)
{
	m_value = value;
	if (m_valueSetAction != nullptr) m_valueSetAction(m_value);
}
void CheckboxGUI::ToggleValue()
{
	SetValue(!IsChecked());
}

void CheckboxGUI::SetValueSetAction(const CheckboxAction& action)
{
	m_valueSetAction = action;
}

void CheckboxGUI::Update()
{
	if (!IsInit()) Init();
	if (!IsSelected()) return;

	//Assert(false, std::format("Checkbox selected"));
	std::optional<ScreenPosition> maybePos = GetSelectorManager().GetLastFrameClickedPosition();
	if (maybePos.has_value() && m_lastFrameCheckboxRect.ContainsPos(maybePos.value()))
	{
		ToggleValue();
		Deselect();
	}
}

ScreenPosition CheckboxGUI::Render(const RenderInfo& renderInfo)
{
	const int guiHeight= std::min(renderInfo.m_RenderSize.m_Y, MAX_HEIGHT);
	const int guiWidth = guiHeight;

	Vector2 currentPos= RaylibUtils::ToRaylibVector(renderInfo.m_TopLeftPos);
	currentPos.x += (renderInfo.m_RenderSize.m_X - guiWidth) / 2;

	DrawRectangle(currentPos.x, currentPos.y, guiWidth, guiHeight, m_settings.m_BackgroundColor);
	m_lastFrameCheckboxRect = GUIRect{ ScreenPosition(currentPos.x, currentPos.y), {guiWidth, guiHeight} };
	if (IsChecked())
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