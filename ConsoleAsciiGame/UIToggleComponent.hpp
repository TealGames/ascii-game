#pragma once
#include "raylib.h"
#include <functional>
#include "UISelectableData.hpp"
#include "GUIStyle.hpp"
#include "GUIRect.hpp"

class TextureAsset;

using ToggleAction = std::function<void(bool isChecked)>;
class UIToggleComponent : public UISelectableData
{
private:
	/// <summary>
	/// Represets the state of the toggle with TRUE-> on state, FALSE-> off state
	/// that changes between the two after a click
	/// </summary>
	bool m_isToggled;
	GUIStyle m_settings;
	const TextureAsset* m_overlayTexture;

	ToggleAction m_valueSetAction;
public:

private:
	void DrawOverlayTexture(const float targetWidth, const float targetHeight, const Vector2& topLeftPos);
public:
	UIToggleComponent(const bool& startValue, const GUIStyle& settings, 
		const ToggleAction& valueSetAction=nullptr, const TextureAsset* overlayTexture=nullptr);
	~UIToggleComponent();

	void SetSettings(const GUIStyle& settings);
	void SetOverlayTexture(const TextureAsset& asset);
	bool HasOverlayTexture() const;

	void SetValue(const bool value);
	void ToggleValue();
	bool IsToggled() const;

	void SetValueSetAction(const ToggleAction& action);

	RenderInfo ElementRender(const RenderInfo& renderInfo) override;

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

