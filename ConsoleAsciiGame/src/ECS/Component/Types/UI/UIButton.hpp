#pragma once
#include "Core/UI/UIStyle.hpp"
#include <functional>
#include <string>
#include "Utils/Data/CooldownEvent.hpp"
#include "ECS/Component/Component.hpp"

class UIButton;
class UITextComponent;
class UISelectableData;
using ButtonAction = std::function<void(const UIButton&)>;
namespace ECS { class UIButtonSystem; }

class UIButton : public Component
{
private:
	//ButtonAction m_clickAction;
	UIStyle m_settings;
	UITextComponent* m_textGUI;
	UISelectableData* m_selectable;
public:
	friend class ECS::UIButtonSystem;

	static const char* DEFAULT_TEXT;
	static constexpr float DEFAULT_COOLDOWN = 0;

private:
	UIButton(UISelectableData* selectable, UITextComponent* textComponent, 
		const UIStyle& settings, const std::string& text, float cooldown);
public:
	UIButton();
	UIButton(const UIStyle& settings);
	UIButton(UISelectableData& selectable, UITextComponent& textComponent, 
		const UIStyle& settings, const std::string& text= "", float clickCooldown = DEFAULT_COOLDOWN);

	void AddClickAction(const ButtonAction& action);
	void SetSettings(const UIStyle& settings);

	void SetText(const std::string& text);
	const UITextComponent* GetTextUI() const;

	bool HasCooldown() const;
	bool IsInCooldown() const;
	void SetCooldownTime(float time);

	void Update(const float deltaTime);

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;

	//RenderInfo ElementRender(const RenderInfo& renderInfo) override;
};

