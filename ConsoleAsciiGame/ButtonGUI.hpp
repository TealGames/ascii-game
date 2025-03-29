#pragma once
#include "SelectableGUI.hpp"
#include "IRenderable.hpp"
#include "GUISettings.hpp"
#include <functional>
#include <string>
#include "TextGUI.hpp"

class ButtonGUI;
using ButtonAction = std::function<void(const ButtonGUI&)>;

class ButtonGUI : public SelectableGUI, public IRenderable
{
private:
	ButtonAction m_clickAction;
	float m_cooldownTime;
	//The amount of time in cooldown currently out of total time
	float m_currentCooldownTime;

	GUISettings m_settings;
	TextGUI m_textGUI;

public:

private:
public:
	ButtonGUI(GUISelectorManager* selectorManager, GUISettings& settings, 
		const std::string text= "", const ButtonAction & clickAction = nullptr, const float& cooldownTime = 0);
	void SetClickAction(const ButtonAction& clickAction);

	void Click() override;
	void SetText(const std::string& text);

	bool HasCooldown() const;
	bool IsInCooldown() const;

	void Update(const float& deltaTime);
	ScreenPosition Render(const RenderInfo& renderInfo) override;
};

