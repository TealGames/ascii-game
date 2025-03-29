#include "pch.hpp"
#include "ButtonGUI.hpp"
#include "raylib.h"

ButtonGUI::ButtonGUI(GUISelectorManager* selectorManager, GUISettings& settings, const std::string text, const ButtonAction& clickAction, const float& cooldownTime) :
	SelectableGUI(selectorManager), m_clickAction(clickAction), m_cooldownTime(cooldownTime), m_currentCooldownTime(0),
	m_settings(settings), m_textGUI(text, settings.m_TextSettings)
{

}

void ButtonGUI::SetClickAction(const ButtonAction& clickAction)
{
	m_clickAction = clickAction;
}

void ButtonGUI::Click()
{
	if (m_clickAction != nullptr)
	{
		m_clickAction(*this);
	}
	//Since a cooldown is detected when there is some time in cooldonw
	//we start this off by giving a little boost for the time
	if (HasCooldown())
	{
		m_currentCooldownTime = 0.01;
	}
}
void ButtonGUI::SetText(const std::string& text)
{
	m_textGUI.SetText(text);
}

bool ButtonGUI::HasCooldown() const
{
	return m_cooldownTime > 0;
}
bool ButtonGUI::IsInCooldown() const
{
	return m_currentCooldownTime > 0 
		&& m_currentCooldownTime < m_cooldownTime;
}

void ButtonGUI::Update(const float& deltaTime)
{
	if (!IsInit()) Init();

	if (IsInCooldown())
	{
		m_currentCooldownTime += deltaTime;
		if (m_currentCooldownTime >= m_cooldownTime)
		{
			m_currentCooldownTime = 0;
		}

		return;
	}
	
	if (!IsSelected()) return;
}

ScreenPosition ButtonGUI::Render(const RenderInfo& renderInfo)
{
	DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y, renderInfo.m_RenderSize.m_X, 
		renderInfo.m_RenderSize.m_Y, m_settings.m_BackgroundColor);

	m_textGUI.Render(renderInfo);
	return renderInfo.m_RenderSize;
}
