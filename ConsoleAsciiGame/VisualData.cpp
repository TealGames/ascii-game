#include "pch.hpp"
#include "VisualData.hpp"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"
#include "raylib.h"
#include "Debug.hpp"

const float& VisualData::DEFAULT_FONT_SIZE = GLOBAL_FONT_SIZE;
const Vec2& VisualData::DEFAULT_CHAR_SPACING = GLOBAL_CHAR_SPACING;
const Vec2& VisualData::DEFAULT_PREDEFINED_CHAR_AREA = GLOBAL_CHAR_AREA;

const Vec2 VisualData::PIVOT_TOP_LEFT = {0, 1};
const Vec2 VisualData::PIVOT_TOP_RIGHT = {1, 1};
const Vec2 VisualData::PIVOT_BOTTOM_LEFT = {0, 0};
const Vec2 VisualData::PIVOT_BOTTOM_RIGHT = { 1, 0 };
const Vec2 VisualData::PIVOT_CENTER = {0.5, 0.5};
const Vec2 VisualData::PIVOT_BOTTOM_CENTER = {0.5, 0};
const Vec2 VisualData::PIVOT_TOP_CENTER = {0.5, 1};

const Vec2 VisualData::DEFAULT_PIVOT = PIVOT_TOP_LEFT;

VisualDataPreset::VisualDataPreset(const Font& font, const float& fontSize, const Vec2& charSpacing,
	const CharAreaType& charAreaType, const Vec2& predefinedCharArea, const NormalizedPosition& relativePivotPos) :
	m_Font(&font), m_FontSize(fontSize), m_CharSpacing(charSpacing), m_CharAreaType(charAreaType), 
		m_PredefinedCharArea(predefinedCharArea), m_RelativePivotPos(relativePivotPos) {}

VisualData::VisualData(const RawTextBufferBlock& rawBuffer, const Font& font, const float& fontSize, 
	const Vec2& charSpacing, const NormalizedPosition& relativePivotPos, const CharAreaType& charAreaType,
	const Vec2& predefinedCharArea) :
	m_Text(), m_font(&font), m_fontSize(fontSize), m_charSpacing(charSpacing), m_pivotRelative(relativePivotPos),
	m_charAreaType(charAreaType), m_predefinedCharArea(predefinedCharArea)
{
	if (rawBuffer.empty()) return;

	std::optional<TextArray> squaredBuffer = CreateSquaredBuffer(rawBuffer);
	if (!Assert(squaredBuffer.has_value(), std::format("Tried to create a Visual data with raw buffer: {}, "
		"but failed to square data", ToStringRawBuffer(rawBuffer)))) return;
	//Log(std::format("Square buffer val in optional: {}", squaredBuffer.value().ToString()));

	m_Text = squaredBuffer.value();
}

VisualData::VisualData() : VisualData(RawTextBufferBlock(), GetGlobalFont(), DEFAULT_FONT_SIZE, DEFAULT_CHAR_SPACING, DEFAULT_PIVOT) {}
VisualData::VisualData(const RawTextBufferBlock& rawBuffer, const Font& font, 
	const float& fontSize, const Vec2& charSpacing, const NormalizedPosition& relativePivotPos)
	: VisualData(rawBuffer, font, fontSize, charSpacing, relativePivotPos, CharAreaType::Adaptive, {})
{
	
}

VisualData::VisualData(const RawTextBufferBlock& rawBuffer, const Font& font,
	const float& fontSize, const Vec2& charSpacing,
	const Vec2& predefinedCharArea, const NormalizedPosition& relativePivotPos)
	: VisualData(rawBuffer, font, fontSize, charSpacing, relativePivotPos, CharAreaType::Predefined, predefinedCharArea)
{

}

VisualData::VisualData(const RawTextBufferBlock& rawBuffer, const VisualDataPreset& preset) : 
	VisualData(rawBuffer, *preset.m_Font, preset.m_FontSize, preset.m_CharSpacing, 
		preset.m_RelativePivotPos, preset.m_CharAreaType, preset.m_PredefinedCharArea)
{

}

bool VisualData::HasValidFont() const
{
	return Assert(this, RaylibUtils::IsValidFont(GetFont()), std::format("Tried to retrieve font for visual data: {} "
		"but the font is not valid (could be due to not being loaded properly)", m_Text.ToString()));
}

////TODO: this function handles the scale in the final camera viewport which makes no sense
////it should intead just 
//void VisualData::TryUpdateTexture()
//{
//	if (!m_doesTextureNeedUpdating) return;
//
//	//TOOD: we do not set this to true when we update the text since that is a puplic member
//
//	Log("Trying to create texture");
//	m_textureSize = {m_charSpacing.m_X * m_Text.GetWidth()+1, m_charSpacing.m_Y * m_Text.GetHeight() +1};
//	int textureWidth = 2 * m_charSpacing.m_X;
//	int textureHeight = 0;
//	Vector2 currentRowSize = {};
//	for (int r = 0; r < m_Text.GetWidth(); r++)
//	{
//		currentRowSize= MeasureTextEx(m_font, m_Text.GetStringAt(r).c_str(), m_fontSize, m_charSpacing.m_X);
//		textureWidth += currentRowSize.x;
//		textureHeight += currentRowSize.y + m_charSpacing.m_Y;
//	}
//	textureHeight += m_charSpacing.m_Y;;
//
//	RenderTexture2D frameBuffer = LoadRenderTexture(textureWidth, textureHeight);
//
//	BeginTextureMode(frameBuffer);
//	ClearBackground(WHITE);
//
//	char drawStr[2] = { '1', '\0' };
//	Array2DPosition bufferPos = {};
//
//	//TODO: maybe the initial offset should not be just the regular char spacing?
//	Vector2 drawPos = {m_charSpacing.m_X, m_charSpacing.m_Y};
//
//	for (int r = 0; r < m_Text.GetHeight(); r++)
//	{
//		bufferPos.SetRow(r);
//		
//		for (int c = 0; c < m_Text.GetWidth(); c++)
//		{
//			bufferPos.SetCol(c);
//			const TextChar& text = m_Text.GetAtUnsafe(bufferPos);
//			drawStr[0] = text.m_Char;
//
//			//TODO: speed up could be to instead draw whole row at a time and then use spacing between chars
//			DrawTextEx(m_font, drawStr, drawPos, m_fontSize, 0, text.m_Color);
//		}
//	}
//	
//	// End drawing to the render target
//	EndTextureMode();
//}

//const Texture2D& VisualData::GetTexture()
//{
//	if (m_doesTextureNeedUpdating) TryUpdateTexture();
//	return m_texture;
//}

std::optional<TextArray> VisualData::CreateSquaredBuffer(const RawTextBufferBlock& rawBuffer) const
{
	std::vector<std::vector<TextChar>> filledSpaces = {};
	filledSpaces.reserve(rawBuffer.size());

	int maxRowCharLen = 0;
	int adjacentColDiff = 0;

	for (const auto& row : rawBuffer)
	{
		if (row.size() > maxRowCharLen) maxRowCharLen = row.size();

		filledSpaces.push_back({});
		filledSpaces.back().reserve(maxRowCharLen);

		for (int i = 1; i < row.size(); i++)
		{
			filledSpaces.back().push_back(row[i - 1].m_Text);
			adjacentColDiff = row[i].m_RowColPos.GetRow() - row[i - 1].m_RowColPos.GetCol();

			if (adjacentColDiff > 1)
			{
				for (int i = 0; i < adjacentColDiff - 1; i++)
				{
					filledSpaces.back().push_back(TextChar(Color(), EMPTY_CHAR_PLACEHOLDER));
				}
			}
		}
		filledSpaces.back().push_back(row.back().m_Text);
	}
	//Log(std::format("After intercolumn check max len: {} actual chars; {}", std::to_string(maxRowCharLen), TextBuffer::ToString(filledSpaces)));

	for (int r = 0; r < filledSpaces.size(); r++)
	{
		if (filledSpaces[r].size() == maxRowCharLen) continue;
		/*if (!Assert(filledSpaces[r].size() < maxRowCharLen,
			std::format("Tried to get rectangalized scene text for render layer but the current "
				"row size: {} is bigger than the max: {}",
				std::to_string(filledSpaces[r].size()), std::to_string(maxRowCharLen))))
			return std::nullopt;*/

		for (int i = 0; i < filledSpaces[r].size() - maxRowCharLen; i++)
			filledSpaces[r].push_back(TextChar(Color(), EMPTY_CHAR_PLACEHOLDER));
	}
	TextArray result = TextArray(filledSpaces[0].size(), filledSpaces.size(), filledSpaces);
	/*Log(std::format("Size: {}x{} result: {}",
		std::to_string(filledSpaces.size()), std::to_string(filledSpaces[0].size()), result.ToString()));*/
	return result;
}

Vec2 VisualData::GetWorldSize() const
{
	if (!HasValidFont()) return {};

	if (m_charAreaType == CharAreaType::Predefined)
	{
		int width = m_Text.GetWidth();
		int height = m_Text.GetHeight();
		Vec2 size = { width* m_predefinedCharArea.m_X + (width - 1) * m_charSpacing.m_X,
				height* m_predefinedCharArea.m_Y+ (height - 1) * m_charSpacing.m_Y };

		//Log(std::format("Getting bad world size: {} real size: {}", size.ToString(), ({size.m_X * })));
		return size;
	}

	float maxWidth = 0;
	float currentWidth = 0;
	float textureHeight = 0;
	Vector2 currentRowTextSize = {};
	std::string currentRowText = "";
	for (int r = 0; r < m_Text.GetHeight(); r++)
	{
		currentRowText = m_Text.GetStringAt(r);
		if (currentRowText.size() == 1) currentRowText += " ";

		currentRowTextSize= MeasureTextEx(GetFont(), currentRowText.c_str(), m_fontSize, 0);
		currentWidth = currentRowTextSize.x + (m_Text.GetWidth() - 1) * m_charSpacing.m_X;
		Log(std::format("Current width for: {} at: {} ({}) is: {} text sixe: {}", 
			m_Text.ToString(), std::to_string(r), currentRowText, std::to_string(currentWidth), std::to_string(currentRowTextSize.x)));
		if (currentWidth > maxWidth) maxWidth = currentWidth;

		textureHeight += currentRowTextSize.y;
		if (r != 0) textureHeight += m_charSpacing.m_Y;
	}
	Log(std::format("World size for: {} is: {}", m_Text.ToString(), Vec2{ maxWidth, textureHeight }.ToString()));
	return { maxWidth, textureHeight };
}

WorldPosition VisualData::GetTopLeftPos(const WorldPosition& pivotWorldPos, const Vec2& totalSize) const
{
	return { pivotWorldPos.m_X - (m_pivotRelative.GetPos().m_X * totalSize.m_X),
			 pivotWorldPos.m_Y + (1 - m_pivotRelative.GetPos().m_Y) * totalSize.m_Y };
}

void VisualData::AddTextPositionsToBufferPredefined(const WorldPosition& transformPos, TextBufferMixed& buffer) const
{
	if (!HasValidFont()) return;
	if (!Assert(this, m_charAreaType == CharAreaType::Predefined,
		std::format("Tried to add text positions to buffer at transform: {} PREDEFINED "
			"but char area type does not have that setting!", transformPos.ToString()))) 
		return;

	Vec2 totalSize = GetWorldSize();
	WorldPosition startTopLeft = GetTopLeftPos(transformPos, totalSize);
	WorldPosition currentTopLeft = startTopLeft;

	Vec2Int charArrSize = m_Text.GetSize();
	for (int r = 0; r < charArrSize.m_Y; r++)
	{
		currentTopLeft.m_X = startTopLeft.m_X;
		for (int c = 0; c < charArrSize.m_X; c++)
		{
			const TextChar& textChar = m_Text.GetAtUnsafe({ r, c });
			buffer.emplace_back(currentTopLeft, textChar, *m_font, m_fontSize);
			currentTopLeft.m_X += (m_predefinedCharArea.m_X + m_charSpacing.m_X);
		}

		currentTopLeft.m_Y -= (m_predefinedCharArea.m_Y + m_charSpacing.m_Y);
	}
}

void VisualData::AddTextPositionsToBufferAdaptive(const WorldPosition& transformPos, TextBufferMixed& buffer) const
{
	if (!HasValidFont()) return;
	if (!Assert(this, m_charAreaType == CharAreaType::Adaptive, 
		std::format("Tried to add text positions to buffer at transform: {} ADPATIVE "
		"but char area type does not have that setting!", transformPos.ToString()))) 
		return;

	Vec2 totalSize = {};
	float currentColsWidth = 0;
	float previousRowsHeight = 0;
	float currentRowMaxHeight = 0;
	Vector2 currentSize = {};

	float maxColHeight = 0;
	char maxColHeightChar = '1';

	char currentTextChar[2] = { '1', '\0' };
	std::vector<TextBufferPosition> textBufferData = {};

	//First pass will calculate distance from top left corner of the text using relative coords
	Vec2Int charArrSize = m_Text.GetSize();
	for (int r = 0; r < charArrSize.m_Y; r++)
	{
		currentRowMaxHeight = 0;
		currentColsWidth = 0;

		maxColHeight = 0;

		for (int c = 0; c < charArrSize.m_X; c++)
		{
			currentTextChar[0] = m_Text.GetAtUnsafe({ r, c }).m_Char;
			currentSize = MeasureTextEx(GetFont(), currentTextChar, m_fontSize, 0);
			Log(std::format("TEXT CHAR SIZE Char: {} of font size: {} at r: {} c: {} has size: {}", Utils::ToString(currentTextChar[0]),
				std::to_string(m_fontSize), std::to_string(r), std::to_string(c), RaylibUtils::ToString(currentSize)));
			//currentSize.y = m_font->baseSize;
			if (currentSize.y > currentRowMaxHeight)
			{
				currentRowMaxHeight = currentSize.y;
				maxColHeight = c;
				maxColHeightChar = currentTextChar[0];
			}

			if (c == 0)
			{
				textBufferData.push_back(TextBufferPosition{ Vec2{ 0, -previousRowsHeight },
					m_Text.GetAtUnsafe({r, c}), *m_font, m_fontSize });
			}

			//We do this here even for when c == width-1 so we can use be added towards total
			currentColsWidth += (currentSize.x + m_charSpacing.m_X);

			if (c < charArrSize.m_X - 1 && charArrSize.m_X >= 2)
			{
				//Note: we get plus one for col since positions is top left by calculating all previous width/height data
				textBufferData.push_back(TextBufferPosition{ Vec2{ currentColsWidth, -previousRowsHeight},
				m_Text.GetAtUnsafe({r, c + 1}), *m_font, m_fontSize });
			}
		}

		Log(std::format("Tried to make visual data now at row: {} has max colL: {} with char: {} of hieght: {}",
			std::to_string(r), std::to_string(maxColHeight), Utils::ToString(maxColHeightChar), std::to_string(currentRowMaxHeight)));

		//Update the current total size based on the stored previous row and col sizes
		previousRowsHeight += (currentRowMaxHeight + m_charSpacing.m_Y);
		totalSize.m_Y = previousRowsHeight;
		if (currentColsWidth > totalSize.m_X) totalSize.m_X = currentColsWidth;
	}
	//Log(std::format("World size for: {} is: {}", m_Text.ToString(), totalSize.ToString()));

	WorldPosition topLeftPos = GetTopLeftPos(transformPos, totalSize);

	//Second pass will add the top left position calculated based on total size from first pass
	for (auto& textBuffer : textBufferData)
	{
		textBuffer.m_Pos.m_X += topLeftPos.m_X;
		textBuffer.m_Pos.m_Y += topLeftPos.m_Y;
		/*Log(std::format("Visual TRANSFORM POS: {} total size: {} top left:{} has buffer: {} ",
			transformPos.ToString(), totalSize.ToString(), topLeftPos.ToString(), textBuffer.ToString()));*/
	}

	for (const auto& textBuffer : textBufferData)
	{
		buffer.emplace_back(textBuffer);
	}
}

void VisualData::AddTextPositionsToBuffer(const WorldPosition& transformPos, TextBufferMixed& buffer) const
{
	if (m_charAreaType == CharAreaType::Predefined) AddTextPositionsToBufferPredefined(transformPos, buffer);
	else if (m_charAreaType == CharAreaType::Adaptive) AddTextPositionsToBufferAdaptive(transformPos, buffer);
	else
	{
		LogError(this, std::format("Tried to add text positions to buffer at transform: {} "
			"but char area tyoe has no actions defiend", transformPos.ToString()));
	}
}

const Vec2& VisualData::GetCharSpacing() const
{
	return m_charSpacing;
}

const Font& VisualData::GetFont() const
{
	if (!Assert(this, m_font != nullptr,
		std::format("Tried to get font for visual but the font is NULL")))
	{
		throw new std::invalid_argument("Invalid font member on VisualData");
	}
		
	return *m_font;
}
const float VisualData::GetFontSize() const
{
	return m_fontSize;
}

const Vec2& VisualData::GetPivot() const
{
	return m_pivotRelative.GetPos();
}

std::string VisualData::ToStringRawBuffer(const RawTextBufferBlock& block)
{
	std::string result = "\nRaw Buffer:\n";
	for (const auto& row : block)
	{
		for (const auto& charPos : row)
		{
			result += charPos.ToString() + " ";
		}
		result += "\n";
	}
	return result;
}
