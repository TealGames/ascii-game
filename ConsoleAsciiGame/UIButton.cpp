#include "pch.hpp"
#include "UIButton.hpp"
#include "raylib.h"
#include "Debug.hpp"
#include "EntityData.hpp"
#include "UITextComponent.hpp"

UIButton::UIButton() : UIButton(UIStyle()) {}
UIButton::UIButton(const UIStyle& settings, const std::string text, const float cooldownTime) :
	UISelectableData(cooldownTime), m_textGUI(nullptr), m_settings(settings)
{
	//m_OnClick.AddListener([this](UISelectableData* self)-> void {InvokeClick(); });
}
void UIButton::AddClickAction(const ButtonAction& action)
{
	m_OnClick.AddListener([this, action](UISelectableData* ptr) -> void { action(*this); });
}
void UIButton::SetSettings(const UIStyle& settings)
{
	m_settings = settings;
	if (m_textGUI!=nullptr) m_textGUI->SetSettings(m_settings.m_TextSettings);
}

const UITextComponent* UIButton::GetTextUI() const { return m_textGUI; }
void UIButton::SetText(const std::string& text)
{
	if (m_textGUI == nullptr)
	{
		Assert(this, false, std::format("Attempted to set text to:{} "
			"for button UI component on entity:{}", text, GetEntity().ToString()));
		return;
	}
	m_textGUI->SetText(text);
}

bool UIButton::HasCooldown() const
{
	return m_OnClick.HasCooldown();
}
bool UIButton::IsInCooldown() const
{
	return m_OnClick.IsInCooldown();
}

void UIButton::Update(const float deltaTime)
{
	m_OnClick.Update(deltaTime);
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