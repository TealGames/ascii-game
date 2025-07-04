#include "pch.hpp"
#include "ECS/Component/Types/UI/UIButton.hpp"
#include "raylib.h"
#include "Core/Analyzation/Debug.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "ECS/Component/Types/UI/UISelectableData.hpp"

const char* UIButton::DEFAULT_TEXT = "Button";

UIButton::UIButton(UISelectableData* selectable, UITextComponent* textComponent, 
	const UIStyle& settings, const std::string& text, float clickCooldown) :
	m_textGUI(textComponent), m_selectable(selectable), m_settings(settings)
{
	if (m_textGUI != nullptr) SetText(text);
	if (m_selectable != nullptr) SetCooldownTime(clickCooldown);
}

UIButton::UIButton() : UIButton(UIStyle()) {}
UIButton::UIButton(const UIStyle& settings) : UIButton(nullptr, nullptr, settings, DEFAULT_TEXT, DEFAULT_COOLDOWN) {}
UIButton::UIButton(UISelectableData& selectable, UITextComponent& textComponent,
	const UIStyle& settings, const std::string& text, float clickCooldown)
	: UIButton(&selectable, &textComponent, settings, text, clickCooldown) {}

void UIButton::AddClickAction(const ButtonAction& action)
{
	m_selectable->m_OnClick.AddListener([this, action](UISelectableData* ptr) -> void { action(*this); });
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
		Assert(false, std::format("Attempted to set text to:{} "
			"for button UI component on entity:{}", text, GetEntity().ToString()));
		return;
	}
	m_textGUI->SetText(text);
}

bool UIButton::HasCooldown() const
{
	return m_selectable->m_OnClick.HasCooldown();
}
bool UIButton::IsInCooldown() const
{
	return m_selectable->m_OnClick.IsInCooldown();
}
void UIButton::SetCooldownTime(float time)
{
	m_selectable->m_OnClick.SetCooldownTime(time);
}

void UIButton::Update(const float deltaTime)
{
	m_selectable->m_OnClick.Update(deltaTime);
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