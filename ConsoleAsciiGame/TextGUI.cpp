#include "pch.hpp"
#include "TextGUI.hpp"
#include "ScreenPosition.hpp"
#include "limits"
#include "RaylibUtils.hpp"
#include "Debug.hpp"
#include "TextGUISettings.hpp"

const GUIPadding TextGUI::DEFAULT_PADDING = GUIPadding();
static constexpr float FONT_SIZE_CALC_DELTA = 0.5;
static constexpr bool DRAW_RENDER_BOUNDS = false;

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

TextGUI::TextGUI() : TextGUI("", {}, Color()) {}

TextGUI::TextGUI(const std::string text, const FontProperties& font, const GUIPadding& padding, 
	const TextAlignment& alignment, const Color& color, const float& factor, const bool& fitToArea) :
	m_text(text), m_fontData(font), m_padding(padding), 
	m_alignment(alignment), m_color(color), m_fontSizeFactor(factor), m_fitToArea(fitToArea) {}

TextGUI::TextGUI(const std::string text, const FontProperties& font, const Color& color) :
	TextGUI(text, font, DEFAULT_PADDING, DEFAULT_ALIGNMENT, color, NULL_FONT_FACTOR, DEFAULT_FIT_TO_AREA) {}
	
TextGUI::TextGUI(const std::string& text, const TextGUIStyle& settings) :
	TextGUI(text, settings.m_FontData, settings.m_Padding, settings.m_TextAlignment,
		settings.m_TextColor, settings.m_FontSizeFactor, settings.m_FitToArea) {}

void TextGUI::SetSettings(const TextGUIStyle& settings)
{
	m_fontData = settings.m_FontData;
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

	m_fontData.m_Size = std::abs(size);
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
	return m_fontData.m_Size;
}
float TextGUI::GetFontSizeFromArea(const Vec2& parentArea, const int textSize) const
{
	const float fontByHeight = parentArea.m_Y * m_fontSizeFactor;

	const float averageCharWidthFactor = 0.6f;
	const float fontByWidth = (parentArea.m_X * m_fontSizeFactor) / (textSize * averageCharWidthFactor);

	return std::min(fontByWidth, fontByHeight);
	//Typically, area that a font size takes up grows with the square of font size
	/*return std::sqrtf(parentArea.m_X * parentArea.m_Y * m_fontSizeFactor);*/
}
Vector2 TextGUI::CalculateSpaceUsed(const float& fontSize, const float& spacing) const
{
	return MeasureTextEx(m_fontData.m_FontType, m_text.c_str(), fontSize, spacing);
}
float TextGUI::CalculateMaxFontSizeForSpace(const Vec2& space, const float spacing, const float startingSize) const
{
	Vector2 currentSpace = CalculateSpaceUsed(startingSize, spacing);
	const bool findingHigherFont = currentSpace.x < space.m_X && currentSpace.y < space.m_Y;

	float fontSize = currentSpace.y;
	if (findingHigherFont)
	{
		while (currentSpace.x < space.m_X && currentSpace.y < space.m_Y)
		{
			fontSize += FONT_SIZE_CALC_DELTA;
			currentSpace = CalculateSpaceUsed(fontSize, spacing);
		}
		//Since the font size that we return with is the one that first surpasses size, we get previous one
		fontSize -= FONT_SIZE_CALC_DELTA;
	}
	else
	{
		while (currentSpace.x > space.m_X || currentSpace.y > space.m_Y)
		{
			fontSize -= FONT_SIZE_CALC_DELTA;
			currentSpace = CalculateSpaceUsed(fontSize, spacing);
		}
	}
	
	return fontSize;
}
float TextGUI::CalculateMaxFontSizeForSpace(const ScreenPosition& space, 
	const float spacing, const float startingSize) const
{
	return CalculateMaxFontSizeForSpace(Vec2(static_cast<float>(space.m_X), 
		static_cast<float>(space.m_Y)), spacing, startingSize);
}
ScreenPosition TextGUI::CalculateTopLeftPos(const RenderInfo& renderInfo, const Vector2& textRectArea) const
{
	int newX = renderInfo.m_TopLeftPos.m_X + m_padding.m_Left;
	int newY = renderInfo.m_TopLeftPos.m_Y + m_padding.m_Top;

	const Vec2 usableSpace = CalculateUsableSpace(renderInfo);
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

Vec2 TextGUI::CalculateUsableSpace(const RenderInfo& renderInfo) const
{
	return {renderInfo.m_RenderSize.m_X - m_padding.m_Left - m_padding.m_Right,
			renderInfo.m_RenderSize.m_Y - m_padding.m_Top - m_padding.m_Bottom};
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

void TextGUI::Update(const float deltaTime) {};

//NOTE: default sizing policy of text is to find size based on settings and if it fits within space to leave it
//but it if does not, will shrink to max posible value that can fit within space
RenderInfo TextGUI::Render(const RenderInfo& renderInfo)
{
	if (m_text.empty()) return {};

	const Vec2 usableSize = CalculateUsableSpace(renderInfo);
	if (HasFontSizeFactor()) m_fontData.m_Size = GetFontSizeFromArea(usableSize, m_text.size());

	Vector2 spaceUsed = CalculateSpaceUsed(m_fontData.m_Size, m_fontData.m_Tracking);
	/*if (m_text == "AssetEditors")
	{
		Assert(false, std::format("Space used:{} font size:{} usable:{} factor:{}", RaylibUtils::ToString(spaceUsed), 
			std::to_string(m_fontData.m_Size), usableSize.ToString(), std::to_string(m_fontSizeFactor)));
	}*/

	if (m_fontData.m_Size==0 || spaceUsed.x > usableSize.m_X || spaceUsed.y > usableSize.m_Y)
	{
		if (m_fontData.m_Size == 0) LogError("Calculing new font size because size is 0");
		m_fontData.m_Size = CalculateMaxFontSizeForSpace(usableSize, m_fontData.m_Tracking, m_fontData.m_Size);
		spaceUsed = CalculateSpaceUsed(m_fontData.m_Size, m_fontData.m_Tracking);
	}

	if (!Assert(this, m_fontData.m_Size != 0, std::format("Tried to render text GUI with id:{} "
		"but font size was calculated to be 0:{} valid font:{}. Usaable space:{} (total space:{}) space used:{}", std::to_string(GetId()),
		ToString(), std::to_string(RaylibUtils::IsValidFont(m_fontData.m_FontType)), renderInfo.m_RenderSize.ToString(), 
		usableSize.ToString(), RaylibUtils::ToString(spaceUsed))))
		return {};

	const Vector2 topLeftPos = RaylibUtils::ToRaylibVector(CalculateTopLeftPos(renderInfo, spaceUsed));
	/*if (m_text=="player") Assert(false, std::format("drawing player text topL:{} actual:{} usablespace:{} reserved:{}", renderInfo.m_TopLeftPos.ToString(),
		RaylibUtils::ToString(topLeftPos), usableSize.ToString(), renderInfo.m_RenderSize.ToString()));*/

	/*Assert(false, std::format("Drawing text gui at:{} size:{} color:{}", RaylibUtils::ToString(topLeftPos),
		std::to_string(m_fontData.m_FontSize), RaylibUtils::ToString(m_color)));*/

	/*if (m_text == "AssetEditors")
	{
		Assert(false, std::format("Creating text total size:{} usabler:{} font size:{} font from area (font factor:{}) :{} space used : {}",
			renderInfo.m_RenderSize.ToString(), usableSize.ToString(), std::to_string(m_fontData.m_Size), 
			std::to_string(HasFontSizeFactor()), std::to_string(GetFontSizeFromArea(usableSize)), RaylibUtils::ToString(spaceUsed)));
	}*/
	LogError(std::format("started drawing test"));
	DrawTextEx(m_fontData.m_FontType, m_text.c_str(), topLeftPos, m_fontData.m_Size, m_fontData.m_Tracking, m_color);
	LogError(std::format("Finsihed drawing test"));
	if (DRAW_RENDER_BOUNDS) DrawRectangleLines(topLeftPos.x, topLeftPos.y, spaceUsed.x, spaceUsed.y, YELLOW);
	//Note: although we use a different top left pos for actual text due to padding, the full object starts at the render info top left
	return renderInfo;
}

ScreenPosition TextGUI::CalculateSize(const RenderInfo& renderInfo) const
{
	if (m_text.empty()) return {};

	const Vec2 usableSize = CalculateUsableSpace(renderInfo);
	float fontSize = HasFontSizeFactor()? GetFontSizeFromArea(usableSize, m_text.size()) : m_fontData.m_Size;

	Vector2 spaceUsed = CalculateSpaceUsed(fontSize, m_fontData.m_Tracking);
	if (spaceUsed.x > usableSize.m_X || spaceUsed.y > usableSize.m_Y)
	{
		fontSize = CalculateMaxFontSizeForSpace(usableSize, m_fontData.m_Tracking, fontSize);
		spaceUsed = CalculateSpaceUsed(fontSize, m_fontData.m_Tracking);
	}
	return {static_cast<int>(spaceUsed.x), static_cast<int>(spaceUsed.y)};
}

std::string TextGUI::ToString() const
{
	return std::format("[TextGUI Text:{} FontSize:{} FontFactor:{} FitToArea:{}]", m_text, std::to_string(GetFontSize()), 
		std::to_string(m_fontSizeFactor), std::to_string(m_fitToArea));
}