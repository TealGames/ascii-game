#include "pch.hpp"
#include "ECS/Component/Types/UI/UIButtonComponent.hpp"
#include "Utils/Debug.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "ECS/Component/Types/UI/UISelectableComponent.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::UI
{
	const char* UIButtonComponent::DEFAULT_TEXT = "Button";

	UIButtonComponent::UIButtonComponent(UISelectableComponent* selectable, UITextComponent* textComponent,
		const UIStyle& settings, const std::string& text, float clickCooldown) :
		m_textGUI(textComponent), m_selectable(selectable), m_settings(settings)
	{
		if (m_textGUI != nullptr) SetText(text);
		if (m_selectable != nullptr) SetCooldownTime(clickCooldown);
	}

	UIButtonComponent::UIButtonComponent() : UIButtonComponent(UIStyle()) {}
	UIButtonComponent::UIButtonComponent(const UIStyle& settings) : UIButtonComponent(nullptr, nullptr, settings, DEFAULT_TEXT, DEFAULT_COOLDOWN) {}
	UIButtonComponent::UIButtonComponent(UISelectableComponent& selectable, UITextComponent& textComponent,
		const UIStyle& settings, const std::string& text, float clickCooldown)
		: UIButtonComponent(&selectable, &textComponent, settings, text, clickCooldown) {}

	void UIButtonComponent::AddClickAction(const ButtonAction& action)
	{
		m_selectable->m_OnClick.AddListener([this, action](UISelectableComponent* ptr) -> void { action(*this); });
	}
	void UIButtonComponent::SetSettings(const UIStyle& settings)
	{
		m_settings = settings;
		if (m_textGUI != nullptr) m_textGUI->SetSettings(m_settings.m_TextSettings);
	}

	const UITextComponent* UIButtonComponent::GetTextUI() const { return m_textGUI; }
	void UIButtonComponent::SetText(const std::string& text)
	{
		if (m_textGUI == nullptr)
		{
			LogError(std::format("Attempted to set text to:{} "
				"for button UI component on entity:{}", text, GetEntity().ToString()));
			return;
		}
		m_textGUI->SetText(text);
	}

	bool UIButtonComponent::HasCooldown() const
	{
		return m_selectable->m_OnClick.HasCooldown();
	}
	bool UIButtonComponent::IsInCooldown() const
	{
		return m_selectable->m_OnClick.IsInCooldown();
	}
	void UIButtonComponent::SetCooldownTime(float time)
	{
		m_selectable->m_OnClick.SetCooldownTime(time);
	}

	void UIButtonComponent::Update(const float deltaTime)
	{
		m_selectable->m_OnClick.Update(deltaTime);
	}

	void UIButtonComponent::InitFields()
	{
		m_Fields = {};
	}
	void UIButtonComponent::Serialize(Serialization::Serializer& serializer) const
	{
	}
	void UIButtonComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
	}
	std::string UIButtonComponent::ToString() const
	{
		return std::format("[ButtonGUI]");
	}
}
