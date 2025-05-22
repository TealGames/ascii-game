#include "pch.hpp"
#include "ToggleGUI.hpp"
#include "RaylibUtils.hpp"
#include "GUISelectorManager.hpp"
#include <optional>

//ToggleGUI::ToggleGUI() : 
//	SelectableGUI(nullptr), m_isToggled(false), m_settings(), 
//	m_valueSetAction(nullptr) {}

ToggleGUI::ToggleGUI(const bool& startValue, const GUIStyle& settings, 
	const ToggleAction& valueSetAction)
	: SelectableGUI(),  m_isToggled(startValue), m_settings(settings), 
	m_valueSetAction(valueSetAction)
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

RenderInfo ToggleGUI::Render(const RenderInfo& renderInfo)
{
	const int guiHeight = renderInfo.m_RenderSize.m_Y;
	const int guiWidth = guiHeight;

	Vector2 topLeftPos= RaylibUtils::ToRaylibVector(renderInfo.m_TopLeftPos);
	topLeftPos.x += (renderInfo.m_RenderSize.m_X - guiWidth) / 2;

	DrawRectangle(topLeftPos.x, topLeftPos.y, guiWidth, guiHeight, m_settings.m_BackgroundColor);
	if (IsToggled())
	{
		const int radius = std::min(guiWidth, guiHeight) / 2;
		DrawCircle(topLeftPos.x+radius, topLeftPos.y+radius, radius, WHITE);
	}

	//Since we do not really have different states between focus input and settings (it is done in one go)
	//we can always draw the disabled overlay
	DrawDisabledOverlay({ ScreenPosition{static_cast<int>(topLeftPos.x),
				static_cast<int>(topLeftPos.y) }, ScreenPosition{guiWidth, guiHeight} });

	//SetLastFramneRect(GUIRect{ ScreenPosition(topLeftPos.x, topLeftPos.y), {guiWidth, guiHeight} });
	return renderInfo;
}