#pragma once
#include "Core/UI/UIStyle.hpp"
#include <functional>
#include <string>
#include "Utils/Data/CooldownEvent.hpp"
#include "ECS/Component/Component.hpp"

namespace Engine::UI
{
	class UIButtonComponent;
	class UITextComponent;
	class UISelectableComponent;
	class UIButtonSystem;

	using ButtonAction = std::function<void(const UIButtonComponent&)>;
	class UIButtonComponent : public ECS::Component
	{
	private:
		//ButtonAction m_clickAction;
		UIStyle m_settings;
		UITextComponent* m_textGUI;
		UISelectableComponent* m_selectable;
	public:
		friend class UIButtonSystem;

		static const char* DEFAULT_TEXT;
		static constexpr float DEFAULT_COOLDOWN = 0;

	private:
		UIButtonComponent(UISelectableComponent* selectable, UITextComponent* textComponent,
			const UIStyle& settings, const std::string& text, float cooldown);
	public:
		UIButtonComponent();
		UIButtonComponent(const UIStyle& settings);
		UIButtonComponent(UISelectableComponent& selectable, UITextComponent& textComponent,
			const UIStyle& settings, const std::string& text = "", float clickCooldown = DEFAULT_COOLDOWN);

		void AddClickAction(const ButtonAction& action);
		void SetSettings(const UIStyle& settings);

		void SetText(const std::string& text);
		const UITextComponent* GetTextUI() const;

		bool HasCooldown() const;
		bool IsInCooldown() const;
		void SetCooldownTime(float time);

		void Update(const float deltaTime);

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}
