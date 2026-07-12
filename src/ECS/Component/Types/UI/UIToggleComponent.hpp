#pragma once
#include <functional>
#include "Core/UI/UIStyle.hpp"
#include "Core/UI/UIRect.hpp"
#include "ECS/Component/Component.hpp"
#include "Utils/Data/Event.hpp"

namespace Engine::Rendering { class TextureAsset; }
namespace Engine::UI
{
	class UISelectableComponent;
	class UITextureComponent;
	class UIPanelComponent;
	class UIToggleSystem;

	constexpr bool DEFAULT_TOGGLE_VALUE = false;

	using ToggleAction = std::function<void(bool isChecked)>;
	class UIToggleComponent : public ECS::Component
	{
	private:
		/// <summary>
		/// Represets the state of the toggle with TRUE-> on state, FALSE-> off state
		/// that changes between the two after a click
		/// </summary>
		bool m_isToggled;
		UIStyle m_settings;
		//const TextureAsset* m_overlayTexture;
		UISelectableComponent* m_selectable;
		UITextureComponent* m_onTexture;
		UITextureComponent* m_offTexture;
		UIPanelComponent* m_background;

		//ToggleAction m_valueSetAction;
	public:
		friend class UIToggleSystem;

		Event<void, bool> m_OnValueSet;
	private:
		//void DrawOverlayTexture(const float targetWidth, const float targetHeight, const Vector2& topLeftPos);
		void SetTextureFromState();
		void UpdateStyle();
		void Init();

	public:
		UIToggleComponent(const bool& startValue = DEFAULT_TOGGLE_VALUE, const UIStyle& settings = UIStyle(), UITextureComponent* onTexture = nullptr,
			UITextureComponent* offTexture = nullptr, UIPanelComponent* background = nullptr);
		//const ToggleAction& valueSetAction=nullptr);//, const TextureAsset* overlayTexture=nullptr);
		~UIToggleComponent();

		void SetSettings(const UIStyle& settings);
		void SetStateTextures(UITextureComponent* onTexture, UITextureComponent* offTexture);
		void SetBackground(UIPanelComponent* background);
		//void SetOverlayTexture(const TextureAsset& asset);
		//bool HasOverlayTexture() const;

		void SetValue(const bool value);
		void ToggleValue();
		bool IsToggled() const;

		//void SetValueSetAction(const ToggleAction& action);

		//RenderInfo ElementRender(const RenderInfo& renderInfo) override;

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}

