#include "pch.hpp"
#include "UIButton.hpp"
#include "raylib.h"
#include "Debug.hpp"
#include "EntityData.hpp"

UIButton::UIButton() : UIButton(GUIStyle(), nullptr) {}
UIButton::UIButton(const GUIStyle& settings, UITextComponent* textUI, const std::string text, const ButtonAction& clickAction, const float& cooldownTime) :
	UISelectableData(), m_clickAction(clickAction), m_cooldownTime(cooldownTime), m_currentCooldownTime(0), m_TextGUI(textUI), m_settings(settings)
{
	m_OnClick.AddListener([this](UISelectableData* self)-> void {InvokeClick(); });
}

void UIButton::SetSettings(const GUIStyle& settings)
{
	m_settings = settings;
	if (m_TextGUI!=nullptr) m_TextGUI->SetSettings(m_settings.m_TextSettings);
}

void UIButton::SetClickAction(const ButtonAction& clickAction)
{
	m_clickAction = clickAction;
}

void UIButton::InvokeClick()
{
	//Assert(false, std::format("BUTTON CLICKED"));
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
const UITextComponent* UIButton::GetTextUI() const { return m_TextGUI; }
void UIButton::SetText(const std::string& text)
{
	if (m_TextGUI == nullptr)
	{
		Assert(this, false, std::format("Attempted to set text to:{} "
			"for button UI component on entity:{}", text, GetEntitySafe().ToString()));
		return;
	}
	m_TextGUI->SetText(text);
}

bool UIButton::HasCooldown() const
{
	return m_cooldownTime > 0;
}
bool UIButton::IsInCooldown() const
{
	return m_currentCooldownTime > 0 
		&& m_currentCooldownTime < m_cooldownTime;
}

void UIButton::Update(const float deltaTime)
{
	if (!IsInCooldown()) return;

	//TODO: cooldown should probably be poart of every selectable
	m_currentCooldownTime += deltaTime;
	if (m_currentCooldownTime >= m_cooldownTime)
	{
		m_currentCooldownTime = 0;
	}
}

void UIButton::InitFields()
{
	m_Fields = {};
}
std::string UIButton::ToString() const
{
	return std::format("[ButtonGUI]");
}

void UIButton::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json UIButton::Serialize()
{
	//TODO: implement
	return {};
}