#pragma once
#include "GUIElement.hpp"
#include <string>
#include "FontData.hpp"
#include "raylib.h"
#include "ScreenPosition.hpp"
#include <cstdint>
#include "GUIPadding.hpp"

class TextGUIStyle;

enum class TextAlignment : std::uint8_t
{
	TopLeft=		0,
	TopCenter=		1,
	TopRight=		2,

	CenterLeft=		3,
	Center=			4,
	CenterRight=	5,

	BottomLeft=		6,
	BottomCenter=	7,
	BottomRight=	8,
};

/// <summary>
/// Return true if alignment is the top row in 3x3 grid
/// </summary>
/// <param name="alignment"></param>
/// <returns></returns>
bool IsTopAlignment(const TextAlignment& alignment);
/// <summary>
/// Returns true if alignment is middle row in 3x3 grid
/// </summary>
/// <param name="alignment"></param>
/// <returns></returns>
bool IsMiddleHorizontalAlignment(const TextAlignment& alignment);
/// <summary>
/// Returns true if alignment is bottom row in 3x3 grid
/// </summary>
/// <param name="alignment"></param>
/// <returns></returns>
bool IsBottomAlignment(const TextAlignment& alignment);

/// <summary>
/// Returns true if alignment is left column in 3x3 grid
/// </summary>
/// <param name="alignment"></param>
/// <returns></returns>
bool IsLeftAlignment(const TextAlignment& alignment);
/// <summary>
/// Returns true if alignment is middle column in 3x3 grid
/// </summary>
/// <param name="alignment"></param>
/// <returns></returns>
bool IsMiddleVerticalAlignment(const TextAlignment& alignment);
/// <summary>
/// Returns true if alignment is right column in 3x3 grid
/// </summary>
/// <param name="alignment"></param>
/// <returns></returns>
bool IsRightAlignment(const TextAlignment& alignment);

class TextGUI : public GUIElement
{
private:
	std::string m_text;
	FontProperties m_fontData;
	Color m_color;

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

	GUIPadding m_padding;

public:
	static constexpr float NULL_FONT_FACTOR = 0;
	static constexpr bool DEFAULT_FIT_TO_AREA = true;
	static constexpr TextAlignment DEFAULT_ALIGNMENT = TextAlignment::Center;
	static const GUIPadding DEFAULT_PADDING;

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
	float CalculateMaxFontSizeForSpace(const Vec2& space, const float spacing, const float startingSize= 0) const;
	float CalculateMaxFontSizeForSpace(const ScreenPosition& space, const float spacing, const float startingSize=0) const;
	Vector2 CalculateSpaceUsed(const float& fontSize, const float& spacing) const;

	/// <summary>
	/// Calculates the top left pos based on the text area reserved and the alignment and padding values
	/// </summary>
	/// <param name="renderInfo"></param>
	/// <param name="textRectArea"></param>
	/// <returns></returns>
	ScreenPosition CalculateTopLeftPos(const RenderInfo& renderInfo, const Vector2& fullTextArea) const;

	/// <summary>
	/// Returns the amount of ACTUAL usable space from rendering information
	/// by taking into account padding and other factors
	/// </summary>
	/// <param name="renderInfo"></param>
	/// <returns></returns>
	Vec2 CalculateUsableSpace(const RenderInfo& renderInfo) const;

private:
	TextGUI(const std::string text, const FontProperties& font, const GUIPadding& padding,
		const TextAlignment& alignment, const Color& color, const float& factor, const bool& fitToArea);

public:
	TextGUI();
	TextGUI(const std::string text, const FontProperties& font, const Color& color);
	TextGUI(const std::string& text, const TextGUIStyle& settings);

	void SetSettings(const TextGUIStyle& settings);

	void SetText(const std::string& text);
	const std::string& GetText() const;

	void SetFontSize(const float& size);
	void SetTextColor(const Color color);
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
	Color GetFontColor() const;

	bool DoFitToArea() const;
	void SetFitToArea(const bool& fit);

	void SetAlignment(const TextAlignment& alignment);
	TextAlignment GetAlignment() const;

	void SetPaddingTop(const float& padding);
	void SetPaddingBottom(const float& padding);
	void SetPaddingRight(const float& padding);
	void SetPaddingLeft(const float& padding);
	void SetPadding(const GUIPadding& padding);

	void Update(const float deltaTime) override;
	RenderInfo Render(const RenderInfo& renderInfo) override;
	ScreenPosition CalculateSize(const RenderInfo& renderInfo) const;

	std::string ToString() const;
};

