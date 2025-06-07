#pragma once
#include "UISelectableData.hpp"
#include "IRenderable.hpp"
#include "GUIStyle.hpp"
#include <functional>
#include <string>
#include "UITextComponent.hpp"

class UIButton;
using ButtonAction = std::function<void(const UIButton&)>;

class UIButton : public UISelectableData
{
private:
	ButtonAction m_clickAction;
	float m_cooldownTime;
	//The amount of time in cooldown currently out of total time
	float m_currentCooldownTime;

	GUIStyle m_settings;

	UITextComponent* m_TextGUI;
public:

private:
	void InvokeClick();

public:
	UIButton();
	UIButton(const GUIStyle& settings, UITextComponent* textUI, const std::string text= "", 
		const ButtonAction& clickAction = nullptr, const float& cooldownTime = 0);
	void SetClickAction(const ButtonAction& clickAction);

	void SetSettings(const GUIStyle& settings);

	void SetText(const std::string& text);
	const UITextComponent* GetTextUI() const;

	bool HasCooldown() const;
	bool IsInCooldown() const;

	void Update(const float deltaTime);

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;

	//RenderInfo ElementRender(const RenderInfo& renderInfo) override;
};

