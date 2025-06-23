#pragma once
#include "UISelectableData.hpp"
#include "IRenderable.hpp"
#include "UIStyle.hpp"
#include <functional>
#include <string>
#include "CooldownEvent.hpp"

class UIButton;
class UITextComponent;
using ButtonAction = std::function<void(const UIButton&)>;
namespace ECS { class UIButtonSystem; }

class UIButton : public UISelectableData
{
private:
	//ButtonAction m_clickAction;
	UIStyle m_settings;
	UITextComponent* m_textGUI;
public:
	friend class ECS::UIButtonSystem;

public:
	UIButton();
	UIButton(const UIStyle& settings, const std::string text= "", const float cooldownTime = 0);

	void AddClickAction(const ButtonAction& action);
	void SetSettings(const UIStyle& settings);

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

