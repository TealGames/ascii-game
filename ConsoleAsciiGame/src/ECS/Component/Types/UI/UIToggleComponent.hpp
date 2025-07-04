#pragma once
#include "raylib.h"
#include <functional>
#include "Core/UI/UIStyle.hpp"
#include "Core/UI/UIRect.hpp"
#include "ECS/Component/Component.hpp"
#include "Utils/Data/Event.hpp"

class TextureAsset;
class UISelectableData;
class UITextureData;
namespace ECS { class UIToggleSystem; }

using ToggleAction = std::function<void(bool isChecked)>;
class UIToggleComponent : public Component
{
private:
	/// <summary>
	/// Represets the state of the toggle with TRUE-> on state, FALSE-> off state
	/// that changes between the two after a click
	/// </summary>
	bool m_isToggled;
	UIStyle m_settings;
	//const TextureAsset* m_overlayTexture;
	UISelectableData* m_selectable;
	UITextureData* m_onTexture;
	UITextureData* m_offTexture;

	//ToggleAction m_valueSetAction;
public:
	friend class ECS::UIToggleSystem;

	Event<void, bool> m_OnValueSet;
private:
	//void DrawOverlayTexture(const float targetWidth, const float targetHeight, const Vector2& topLeftPos);
	void SetTextureFromState();
public:
	UIToggleComponent(const bool& startValue, const UIStyle& settings, UITextureData* onTexture=nullptr, UITextureData* offTexture=nullptr);
	//const ToggleAction& valueSetAction=nullptr);//, const TextureAsset* overlayTexture=nullptr);
	~UIToggleComponent();

	void Init();

	void SetSettings(const UIStyle& settings);
	void SetStateTextures(UITextureData* onTexture, UITextureData* offTexture);
	//void SetOverlayTexture(const TextureAsset& asset);
	//bool HasOverlayTexture() const;

	void SetValue(const bool value);
	void ToggleValue();
	bool IsToggled() const;

	//void SetValueSetAction(const ToggleAction& action);

	//RenderInfo ElementRender(const RenderInfo& renderInfo) override;

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

