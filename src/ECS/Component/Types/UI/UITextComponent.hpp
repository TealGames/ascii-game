#pragma once
#include "ECS/Component/Component.hpp"
#include <string>
#include "Core/Rendering/FontData.hpp"
//#include "raylib.h"
#include "Utils/Data/ScreenPosition.hpp"
#include "Core/UI/UIRect.hpp"
#include "Core/UI/UITextStyle.hpp"

class UIRendererData;
class TextUIStyle;
namespace ECS { class UITextSystem; }

class UITextComponent : public Component
{
private:
	UIRendererData* m_renderer;

	std::string m_text;
	ScreenFontProperties m_fontData;
	HDRColor m_color;

	/// <summary>
	/// When is not -1, sets the font of this text 
	/// based off of this factor and an area (the parent) provided
	/// </summary>
	float m_fontSizeFactor;

	/// <summary>
	/// If true, will shrink font size to max possible within an area when font size is too big
	/// to fit all of the text inside the given area during render update. 
	/// Note: font size factors CAN be overriden if this is true
	/// </summary>
	bool m_fitToArea;

	/// <summary>
	/// The alignment of the text within its render area given by render update
	/// Note: since text takes up max space possible, we can easily then align
	/// the text within that area since it usually will not take up max space
	/// If it equals the max space, it will effectively not consider alignment
	/// </summary>
	TextAlignment m_alignment;

	UIPadding m_padding;

public:
	friend class ECS::UITextSystem;

private:
	/// <summary>
	/// Will approximate the best font based on the area given. 
	/// Note: giving a text size greatly improves the accuracy of the estimation
	/// </summary>
	/// <param name="parentArea"></param>
	/// <param name="textSize"></param>
	/// <returns></returns>
	float GetFontSizeFromArea(const Vec2& parentArea, const int textSize= -1) const;
	/// <summary>
	/// Will find the best font size using the spacing and space requirements.
	/// Note: sicne this is an iterative and slow process, starting sizes greatly help in reducing iterations
	/// </summary>
	/// <param name="space"></param>
	/// <param name="spacing"></param>
	/// <param name="startingSize"></param>
	/// <returns></returns>
	float CalculateMaxFontSizeForSpace(const Vec2& space, const float spacing, const float startingSize=0) const;
	Vec2 CalculateSpaceUsed(const float& fontSize, const float& spacing) const;

	/// <summary>
	/// Calculates the top left pos based on the text area reserved and the alignment and padding values
	/// </summary>
	/// <param name="renderInfo"></param>
	/// <param name="textRectArea"></param>
	/// <returns></returns>
	ScreenPosition CalculateTopLeftPos(const UIRect& renderInfo, const Vec2& fullTextArea) const;

private:
	UITextComponent(const std::string text, const ScreenFontProperties& font, const UIPadding& padding,
		const TextAlignment& alignment, const HDRColor& color, const float& factor, const bool& fitToArea);

public:
	UITextComponent();
	UITextComponent(const std::string text, const ScreenFontProperties& font, const HDRColor& color);
	UITextComponent(const std::string& text, const TextUIStyle& settings);

	void SetSettings(const TextUIStyle& settings);

	void SetText(const std::string& text);
	const std::string& GetText() const;

	void SetFontSize(const float& size);
	void SetTextColor(const HDRColor color);
	/// <summary>
	/// Sets the factor of the text relative to the parent area. 
	/// Note: value is clamped to be positive
	/// Note: if this is set at least once before render update, 
	/// this behavior WILL override default font size behavior (even if that value was set)
	/// </summary>
	/// <param name="factor"></param>
	void SetFontFactorSize(const float& factor);
	/// <summary>
	/// Removes any set font size factors set, allowing for default font size
	/// behavior to occur instead of font size factor
	/// </summary>
	void ClearFontSizeFactor();
	bool HasFontSizeFactor() const;

	/// <summary>
	/// Returns the font size of the last render update of this text object
	/// </summary>
	/// <returns></returns>
	float GetFontSize() const;
	HDRColor GetFontColor() const;

	bool DoFitToArea() const;
	void SetFitToArea(const bool& fit);

	void SetAlignment(const TextAlignment& alignment);
	TextAlignment GetAlignment() const;

	void SetPaddingTop(const float& padding);
	void SetPaddingBottom(const float& padding);
	void SetPaddingRight(const float& padding);
	void SetPaddingLeft(const float& padding);
	void SetPadding(const UIPadding& padding);

	//RenderInfo Render(const RenderInfo& renderInfo) override;
	UIRect Render(const UIRect& rect);

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

