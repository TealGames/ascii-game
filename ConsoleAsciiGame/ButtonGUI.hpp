#pragma once
#include "SelectableGUI.hpp"
#include "IRenderable.hpp"
#include "GUIStyle.hpp"
#include <functional>
#include <string>
#include "TextGUI.hpp"

class ButtonGUI;
using ButtonAction = std::function<void(const ButtonGUI&)>;

class ButtonGUI : public SelectableGUI
{
private:
	ButtonAction m_clickAction;
	float m_cooldownTime;
	//The amount of time in cooldown currently out of total time
	float m_currentCooldownTime;

	GUIStyle m_settings;
	TextGUI m_textGUI;

public:

private:
	void InvokeClick();

public:
	ButtonGUI(const GUIStyle& settings, const std::string text= "", 
		const ButtonAction& clickAction = nullptr, const float& cooldownTime = 0);
	void SetClickAction(const ButtonAction& clickAction);

	void SetSettings(const GUIStyle& settings);

	void SetText(const std::string& text);

	bool HasCooldown() const;
	bool IsInCooldown() const;

	void Update(const float deltaTime);
	RenderInfo ElementRender(const RenderInfo& renderInfo) override;
};

