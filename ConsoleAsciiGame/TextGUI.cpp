#include "pch.hpp"
#include "TextGUI.hpp"
#include "ScreenPosition.hpp"
#include "limits"
#include "RaylibUtils.hpp"
#include "Debug.hpp"
#include "TextGUISettings.hpp"

const GUIPadding TextGUI::DEFAULT_PADDING = GUIPadding();

bool IsTopAlignment(const TextAlignment& alignment)
{
	return static_cast<std::uint8_t>(alignment) <= 2;
}
bool IsMiddleHorizontalAlignment(const TextAlignment& alignment)
{
	std::uint8_t integralValue = static_cast<std::uint8_t>(alignment);
	return 3 <= integralValue && integralValue <= 5;
}
bool IsBottomAlignment(const TextAlignment& alignment)
{
	std::uint8_t integralValue = static_cast<std::uint8_t>(alignment);
	return 6 <= integralValue && integralValue <= 8;
}

bool IsLeftAlignment(const TextAlignment& alignment)
{
	std::uint8_t integralValue = static_cast<std::uint8_t>(alignment);
	return integralValue == 0 || integralValue == 3 || integralValue == 6;
}
bool IsMiddleVerticalAlignment(const TextAlignment& alignment)
{
	std::uint8_t integralValue = static_cast<std::uint8_t>(alignment);
	return integralValue == 1 || integralValue == 4 || integralValue == 7;
}
bool IsRightAlignment(const TextAlignment& alignment)
{
	std::uint8_t integralValue = static_cast<std::uint8_t>(alignment);
	return integralValue == 2 || integralValue == 5 || integralValue == 8;
}

TextGUI::TextGUI() : TextGUI("", {}, 0, Color()) {}

TextGUI::TextGUI(const std::string text, const FontData& font, const float& charSpacing, const GUIPadding& padding, 
	const TextAlignment& alignment, const Color& color, const float& factor, const bool& fitToArea) :
	m_text(text), m_fontData(font), m_charSpacing(charSpacing), m_padding(padding), 
	m_alignment(alignment), m_color(color), m_fontSizeFactor(factor), m_fitToArea(fitToArea) {}

TextGUI::TextGUI(const std::string text, const FontData& font, const float& charSpacing, const Color& color) :
	TextGUI(text, font, charSpacing, DEFAULT_PADDING, DEFAULT_ALIGNMENT, color, NULL_FONT_FACTOR, DEFAULT_FIT_TO_AREA) {}
	
TextGUI::TextGUI(const std::string& text, const TextGUISettings& settings) :
	TextGUI(text, settings.m_FontData, settings.m_CharSpacing, settings.m_Padding, settings.m_TextAlignment,
		settings.m_TextColor, settings.m_FontSizeFactor, settings.m_FitToArea) {}

void TextGUI::SetSettings(const TextGUISettings& settings)
{
	m_fontData = settings.m_FontData;
	m_charSpacing = settings.m_CharSpacing;
	m_padding = settings.m_Padding;
	m_alignment = settings.m_TextAlignment;
	m_color = settings.m_TextColor;
	m_fontSizeFactor = settings.m_FontSizeFactor;
	m_fitToArea = settings.m_FitToArea;
}

void TextGUI::SetText(const std::string& text)
{
	m_text = text;
}

const std::string& TextGUI::GetText() const
{
	return m_text;
}
void TextGUI::SetFontSize(const float& size)
{
	if (!Assert(this, size!=0, std::format("Tried to set font size "
		"to:{} which is not allowed", std::to_string(size))))
		return;

	m_fontData.m_FontSize = std::abs(size);
}


void TextGUI::SetFontFactorSize(const float& factor)
{
	if (!Assert(this, m_fontSizeFactor!=NULL_FONT_FACTOR, std::format("Tried to set font factor "
		"size to:{} which is not allowed", std::to_string(factor))))
		return;

	m_fontSizeFactor = std::abs(factor);
}
void TextGUI::ClearFontSizeFactor()
{
	m_fontSizeFactor = NULL_FONT_FACTOR;
}
bool TextGUI::HasFontSizeFactor() const
{
	return m_fontSizeFactor != NULL_FONT_FACTOR;
}


float TextGUI::GetFontSize() const
{
	//Note: no matter what font type it is, font data will always store
	//the last font size used
	return m_fontData.m_FontSize;
}
float TextGUI::GetFontSizeFromArea(const ScreenPosition& parentArea) const
{
	return static_cast<float>(parentArea.m_Y) * m_fontSizeFactor;
}
Vector2 TextGUI::CalculateSpaceUsed(const float& fontSize, const float& spacing) const
{
	return MeasureTextEx(m_fontData.m_Font, m_text.c_str(), fontSize, spacing);
}
float TextGUI::CalculateMaxFontSizeForSpace(const Vec2& space, const float& spacing) const
{
	Vector2 currentSpace = { std::numeric_limits<float>().max(), std::numeric_limits<float>().max() };
	float fontSize = space.m_Y;
	while (currentSpace.x <= space.m_X && currentSpace.y <= space.m_Y)
	{
		currentSpace = CalculateSpaceUsed(fontSize, spacing);
	}
	return fontSize;
}
float TextGUI::CalculateMaxFontSizeForSpace(const ScreenPosition& space, const float& spacing) const
{
	return CalculateMaxFontSizeForSpace(Vec2(static_cast<float>(space.m_X), static_cast<float>(space.m_Y)), spacing);
}
ScreenPosition TextGUI::CalculateTopLeftPos(const RenderInfo& renderInfo, const Vector2& textRectArea) const
{
	int newX = renderInfo.m_TopLeftPos.m_X + m_padding.m_Left;
	int newY = renderInfo.m_TopLeftPos.m_Y + m_padding.m_Top;

	const ScreenPosition usableSpace = CalculateUsableSpace(renderInfo);
	const float xSpaceLeft = usableSpace.m_X - textRectArea.x;
	const float ySpaceLeft = usableSpace.m_Y - textRectArea.y;

	//Note: as you go down, y increases, as you go right x increases
	if (IsMiddleHorizontalAlignment(m_alignment))
	{
		newY += ySpaceLeft / 2;
	}
	else if (IsBottomAlignment(m_alignment))
	{
		newY += ySpaceLeft;
	}

	if (IsMiddleVerticalAlignment(m_alignment))
	{
		newX += xSpaceLeft / 2;
	}
	else if (IsRightAlignment(m_alignment))
	{
		newX += xSpaceLeft;
	}

	return {newX, newY};
}

ScreenPosition TextGUI::CalculateUsableSpace(const RenderInfo& renderInfo) const
{
	return {static_cast<int>(renderInfo.m_RenderSize.m_X - m_padding.m_Left - m_padding.m_Right), 
			static_cast<int>(renderInfo.m_RenderSize.m_Y - m_padding.m_Top - m_padding.m_Bottom)};
}

Color TextGUI::GetFontColor() const
{
	return m_color;
}

bool TextGUI::DoFitToArea() const
{
	return m_fitToArea;
}
void TextGUI::SetFitToArea(const bool& fit)
{
	m_fitToArea = fit;
}

void TextGUI::SetAlignment(const TextAlignment& alignment)
{
	m_alignment = alignment;
}
TextAlignment TextGUI::GetAlignment() const
{
	return m_alignment;
}

void TextGUI::SetPaddingTop(const float& padding)
{
	m_padding.m_Top = padding;
}
void TextGUI::SetPaddingBottom(const float& padding)
{
	m_padding.m_Bottom = padding;
}
void TextGUI::SetPaddingRight(const float& padding)
{
	m_padding.m_Right = padding;
}
void TextGUI::SetPaddingLeft(const float& padding)
{
	m_padding.m_Left = padding;
}
void TextGUI::SetPadding(const GUIPadding& padding)
{
	m_padding = padding;
}

ScreenPosition TextGUI::Render(const RenderInfo& renderInfo)
{
	if (m_text.empty()) return {};

	const ScreenPosition usableSize = CalculateUsableSpace(renderInfo);
	if (HasFontSizeFactor()) m_fontData.m_FontSize = GetFontSizeFromArea(usableSize);

	Vector2 spaceUsed = CalculateSpaceUsed(m_fontData.m_FontSize, m_charSpacing);
	if (m_fontData.m_FontSize==0 || spaceUsed.x > usableSize.m_X || spaceUsed.y > usableSize.m_Y)
	{
		//LogError("Calculing new font size");
		m_fontData.m_FontSize = CalculateMaxFontSizeForSpace(usableSize, m_charSpacing);
		spaceUsed = CalculateSpaceUsed(m_fontData.m_FontSize, m_charSpacing);
	}

	if (!Assert(this, m_fontData.m_FontSize != 0, std::format("Tried to render text GUI "
		"but font size was calcualte to be 0:{} valid font:{}. Usaable space:{} (total space:{}) space used:{}", 
		ToString(), std::to_string(RaylibUtils::IsValidFont(m_fontData.m_Font)), renderInfo.m_RenderSize.ToString(), 
		usableSize.ToString(), RaylibUtils::ToString(spaceUsed))))
		return {};

	const Vector2 topLeftPos = RaylibUtils::ToRaylibVector(CalculateTopLeftPos(renderInfo, spaceUsed));
	/*if (m_text=="player") Assert(false, std::format("drawing player text topL:{} actual:{} usablespace:{} reserved:{}", renderInfo.m_TopLeftPos.ToString(),
		RaylibUtils::ToString(topLeftPos), usableSize.ToString(), renderInfo.m_RenderSize.ToString()));*/

	/*Assert(false, std::format("Drawing text gui at:{} size:{} color:{}", RaylibUtils::ToString(topLeftPos),
		std::to_string(m_fontData.m_FontSize), RaylibUtils::ToString(m_color)));*/

	DrawTextEx(m_fontData.m_Font, m_text.c_str(), topLeftPos, m_fontData.m_FontSize, m_charSpacing, m_color);
	return renderInfo.m_RenderSize;
}

ScreenPosition TextGUI::CalculateSize(const RenderInfo& renderInfo) const
{
	if (m_text.empty()) return {};

	const ScreenPosition usableSize = CalculateUsableSpace(renderInfo);
	float fontSize = 0;
	if (HasFontSizeFactor()) fontSize = GetFontSizeFromArea(usableSize);

	Vector2 spaceUsed = CalculateSpaceUsed(m_fontData.m_FontSize, m_charSpacing);
	if (spaceUsed.x > usableSize.m_X || spaceUsed.y > usableSize.m_Y)
	{
		fontSize = CalculateMaxFontSizeForSpace(usableSize, m_charSpacing);
		spaceUsed = CalculateSpaceUsed(m_fontData.m_FontSize, m_charSpacing);
	}
	return {static_cast<int>(spaceUsed.x), static_cast<int>(spaceUsed.y)};
}

std::string TextGUI::ToString() const
{
	return std::format("[TextGUI Text:{} FontSize:{} FontFactor:{} Spacing:{} FitToArea:{}]", m_text, std::to_string(GetFontSize()), 
		std::to_string(m_fontSizeFactor), std::to_string(m_charSpacing), std::to_string(m_fitToArea));
}