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

const Vec2 VisualData::DEFAULT_PIVOT = PIVOT_CENTER;

VisualDataPreset::VisualDataPreset(const Font& font, const float& fontSize, const Vec2& charSpacing,
	const CharAreaType& charAreaType, const Vec2& predefinedCharArea, const NormalizedPosition& relativePivotPos) :
	m_Font(font), m_FontSize(fontSize), m_CharSpacing(charSpacing), m_CharAreaType(charAreaType), 
		m_PredefinedCharArea(predefinedCharArea), m_RelativePivotPos(relativePivotPos) {}

std::string ToString(const RawTextBufferBlock& rawBuffer)
{
	std::string fullStr = "";
	for (size_t r = 0; r < rawBuffer.size(); r++)
	{
		fullStr += "\n";
		for (size_t c = 0; c < rawBuffer[r].size(); c++)
		{
			fullStr += std::format("[{}] ", rawBuffer[r][c].ToString());
		}
	}
	return fullStr;
}

VisualData::VisualData(const RawTextBufferBlock& rawBuffer, const Font& font, const float& fontSize,
	const Vec2& charSpacing, const NormalizedPosition& relativePivotPos, const CharAreaType& charAreaType,
	const Vec2& predefinedCharArea) :
	m_Text(), m_rawTextBlock(rawBuffer),m_fontData(fontSize, font), m_charSpacing(charSpacing), m_pivotRelative(relativePivotPos),
	m_charAreaType(charAreaType), m_predefinedCharArea(predefinedCharArea)
{
	if (rawBuffer.empty()) return;
	if (!Assert(this, HasValidFont(false), std::format("Tried to create visual data: {} "
		"but the font argument in constructor is INVALID", ToString()))) return;

	LogError(std::format("B egan creating square buffer with: {}", ::ToString(m_rawTextBlock)));

	std::optional<TextArray> squaredBuffer = CreateRectangularBuffer(rawBuffer);
	if (!Assert(squaredBuffer.has_value(), std::format("Tried to create a Visual data with raw buffer: {}, "
		"but failed to square data", ToStringRawBuffer(rawBuffer)))) return;
	//Log(std::format("Square buffer val in optional: {}", squaredBuffer.value().ToString()));

	m_Text = squaredBuffer.value();
}

VisualData::VisualData() : VisualData(RawTextBufferBlock(), GetGlobalFont(), DEFAULT_FONT_SIZE, DEFAULT_CHAR_SPACING, DEFAULT_PIVOT) { }
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
	VisualData(rawBuffer, preset.m_Font, preset.m_FontSize, preset.m_CharSpacing, 
		preset.m_RelativePivotPos, preset.m_CharAreaType, preset.m_PredefinedCharArea)
{
	LogError("Began creating visual data");
}

bool VisualData::HasValidFont(const bool assertMessage, const std::string& extraMessage) const
{
	std::string fullMessage = std::format("Tried to retrieve font for visual data: {} "
		"but the font is not valid (could be due to not being loaded properly)", m_Text.ToString(), extraMessage);
	if (!extraMessage.empty())fullMessage += std::format("Info: {}", extraMessage);

	bool isFontValid = RaylibUtils::IsValidFont(GetFont());
	if (!assertMessage) return isFontValid;

	return Assert(this, isFontValid, fullMessage);
}

std::optional<TextArray> VisualData::CreateRectangularBuffer(const RawTextBufferBlock& rawBuffer) const
{
	std::vector<std::vector<TextChar>> filledSpaces = {};
	filledSpaces.reserve(rawBuffer.size());

	size_t maxWidth = 0;
	Array2DPosition currArrPos = {};

	size_t r = 0;
	size_t c = 0;
	size_t spaceDiff = 0;
	for (r = 0; r < rawBuffer.size(); r++)
	{
		filledSpaces.push_back({});
		if (maxWidth >0) filledSpaces.back().reserve(maxWidth);

		//Note: this section checks to make sure that there are no skips in rows
		if (r != 0)
		{
			//We take the difference between the current row and the last row (we are guaranteed
			//to have at least one element on row above if NOT at row 0
			spaceDiff = rawBuffer[r][0].m_RowColPos.GetRow() - rawBuffer[r - 1][0].m_RowColPos.GetRow();

			//Note: 1 diff is the ideal amount so diff 1-> 0 iterations
			//so if it is greater than we fill extra spaces with empty char placeholder
			for (size_t i = 0; i < spaceDiff - 1; i++)
			{
				filledSpaces.push_back({});
				filledSpaces.reserve(maxWidth);
				for (size_t j = 0; j < maxWidth; j++)
				{
					filledSpaces.back().push_back(TextChar(Color(), EMPTY_CHAR_PLACEHOLDER));
				}
			}
		}

		for (c = 0; c < rawBuffer[r].size(); c++)
		{
			LogError(std::format("Going through r: {} c:{}", std::to_string(r), std::to_string(c)));
			//Note: we must have each non 0 col have the same row as the previous col in order to not mess the process up
			//Note: curreArrPos is used as the previous row col pos since we do this check before the new one is assigned
			if (c != 0 && !Assert(this, currArrPos.GetRow() == rawBuffer[r][c].m_RowColPos.GetRow(),
				std::format("Tried to create rectangular buffer of raw buffer:{} but at raw buffer r:{} c:{} the"
					"pos:{} does not have the same row as the previous one: {}", ::ToString(rawBuffer), std::to_string(r), std::to_string(c),
					rawBuffer[r][c].m_RowColPos.ToString(), currArrPos.ToString())))
				return {};

			currArrPos = rawBuffer[r][c].m_RowColPos;
			spaceDiff = (currArrPos.GetCol() - rawBuffer[r][0].m_RowColPos.GetCol()) + 1;
			//Note: since the first raw col pos may (incorrectly) be a non (0,0) we must subtract to get actual columns
			if (spaceDiff > maxWidth)
			{
				maxWidth = spaceDiff;
				//Assert(false, std::format("Max width updated to:{}", std::to_string(maxWidth)));
			}

			//Note: this section makes suere there are no skips before any columns
			//and MUST be done before any existing text char is added to ensure order is maintained
			if (c != 0)
			{
				spaceDiff = currArrPos.GetCol() - rawBuffer[r][c - 1].m_RowColPos.GetCol();
				for (size_t i = 0; i < spaceDiff - 1; i++)
				{
					filledSpaces.back().push_back(TextChar(Color(), EMPTY_CHAR_PLACEHOLDER));
				}
			}

			filledSpaces.back().push_back(rawBuffer[r][c].m_Text);
		}
	}

	//Since we do not know max width at any definitive point, we need to check they all have 
	//max width and add any extra chars at the end
	spaceDiff = 0;
	for (r=0; r<filledSpaces.size(); r++)
	{
		spaceDiff = maxWidth - filledSpaces[r].size();
		if (!Assert(this, spaceDiff >= 0, std::format("Tried to create rectangular buffer of raw buffer:{} "
			"but when trying to fill remaining columns with max width:{} the amount of space to fill for row:{} minus row filled space:{} is:{} which is not allowed",
			::ToString(rawBuffer), std::to_string(maxWidth), std::to_string(r), std::to_string(filledSpaces[r].size()),
			std::to_string(spaceDiff))))
			return {};

		
		for (c = 0; c < spaceDiff; c++)
		{
			LogError(std::format("Row:{} has width:{} needed:{}", std::to_string(r), std::to_string(filledSpaces[r].size()), std::to_string(maxWidth)));
			filledSpaces[r].push_back(TextChar(Color(), EMPTY_CHAR_PLACEHOLDER));
		}
	}

	LogError(std::format("REACHED CREATION of size:[{}, {}]. buffer:{} rectangular:{}", std::to_string(maxWidth), 
		std::to_string(filledSpaces.size()), ::ToString(rawBuffer), ::ToString(filledSpaces)));
	return TextArray(maxWidth, filledSpaces.size(), filledSpaces);

	//std::vector<std::vector<TextChar>> filledSpaces = {};
	//filledSpaces.reserve(rawBuffer.size());

	//for (r = 0; r < rawBuffer.size(); r++)
	//{
	//	filledSpaces.push_back({});
	//	filledSpaces.back().reserve(maxWidth);

	//	for (c=0; c < rawBuffer[r].size(); c++)
	//	{
	//		filledSpaces.back().push_back(TextChar(Color(), EMPTY_CHAR_PLACEHOLDER));
	//	}
	//}

	//std::vector<std::vector<TextChar>> filledSpaces = {};
	//filledSpaces.reserve(rawBuffer.size());

	//int maxRowCharLen = 0;
	//int adjacentColDiff = 0;

	//for (const auto& row : rawBuffer)
	//{
	//	if (row.size() > maxRowCharLen) maxRowCharLen = row.size();

	//	filledSpaces.push_back({});
	//	filledSpaces.back().reserve(maxRowCharLen);

	//	for (int i = 1; i < row.size(); i++)
	//	{
	//		filledSpaces.back().push_back(row[i - 1].m_Text);
	//		adjacentColDiff = row[i].m_RowColPos.GetRow() - row[i - 1].m_RowColPos.GetCol();

	//		if (adjacentColDiff > 1)
	//		{
	//			for (int i = 0; i < adjacentColDiff - 1; i++)
	//			{
	//				filledSpaces.back().push_back(TextChar(Color(), EMPTY_CHAR_PLACEHOLDER));
	//			}
	//		}
	//	}
	//	filledSpaces.back().push_back(row.back().m_Text);
	//}
	////Log(std::format("After intercolumn check max len: {} actual chars; {}", std::to_string(maxRowCharLen), TextBuffer::ToString(filledSpaces)));

	//for (int r = 0; r < filledSpaces.size(); r++)
	//{
	//	if (filledSpaces[r].size() == maxRowCharLen) continue;
	//	/*if (!Assert(filledSpaces[r].size() < maxRowCharLen,
	//		std::format("Tried to get rectangalized scene text for render layer but the current "
	//			"row size: {} is bigger than the max: {}",
	//			std::to_string(filledSpaces[r].size()), std::to_string(maxRowCharLen))))
	//		return std::nullopt;*/

	//	for (int i = 0; i < filledSpaces[r].size() - maxRowCharLen; i++)
	//		filledSpaces[r].push_back(TextChar(Color(), EMPTY_CHAR_PLACEHOLDER));
	//}
	//TextArray result = TextArray(filledSpaces[0].size(), filledSpaces.size(), filledSpaces);
	///*Log(std::format("Size: {}x{} result: {}",
	//	std::to_string(filledSpaces.size()), std::to_string(filledSpaces[0].size()), result.ToString()));*/
	//return result;
}

const RawTextBufferBlock& VisualData::GetRawBuffer() const
{
	return m_rawTextBlock;
}

Vec2Int VisualData::GetBufferSize() const
{
	return m_Text.GetSize();
}

Vec2 VisualData::GetWorldSize() const
{
	if (!HasValidFont("Attempted to get world size")) return {};

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

		currentRowTextSize= MeasureTextEx(GetFont(), currentRowText.c_str(), GetFontSize(), 0);
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
	if (!HasValidFont("Attempted to add text positions to buffer in predefined mode")) return;
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
			buffer.emplace_back(currentTopLeft, textChar, m_fontData);
			currentTopLeft.m_X += (m_predefinedCharArea.m_X + m_charSpacing.m_X);
		}

		currentTopLeft.m_Y -= (m_predefinedCharArea.m_Y + m_charSpacing.m_Y);
	}
}

void VisualData::AddTextPositionsToBufferAdaptive(const WorldPosition& transformPos, TextBufferMixed& buffer) const
{
	if (!HasValidFont("Attempted to add text positions to buffer in adaptive mode")) return;
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
			currentSize = MeasureTextEx(GetFont(), currentTextChar, GetFontSize(), 0);
			/*Log(std::format("TEXT CHAR SIZE Char: {} of font size: {} at r: {} c: {} has size: {}", Utils::ToString(currentTextChar[0]),
				std::to_string(GetFontSize()), std::to_string(r), std::to_string(c), RaylibUtils::ToString(currentSize)));*/
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
					m_Text.GetAtUnsafe({r, c}), m_fontData });
			}

			//We do this here even for when c == width-1 so we can use be added towards total
			currentColsWidth += (currentSize.x + m_charSpacing.m_X);

			if (c < charArrSize.m_X - 1 && charArrSize.m_X >= 2)
			{
				//Note: we get plus one for col since positions is top left by calculating all previous width/height data
				textBufferData.push_back(TextBufferPosition{ Vec2{ currentColsWidth, -previousRowsHeight},
				m_Text.GetAtUnsafe({r, c + 1}), m_fontData});
			}
		}

		/*Log(std::format("Tried to make visual data now at row: {} has max colL: {} with char: {} of hieght: {}",
			std::to_string(r), std::to_string(maxColHeight), Utils::ToString(maxColHeightChar), std::to_string(currentRowMaxHeight)));*/

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
	return m_fontData.m_Font;
}
float VisualData::GetFontSize() const
{
	return m_fontData.m_FontSize;
}
const FontData& VisualData::GetFontData() const
{
	return m_fontData;
}

const Vec2& VisualData::GetPivot() const
{
	return m_pivotRelative.GetPos();
}

void VisualData::SetPredefinedCharArea(const Vec2& area)
{
	m_charAreaType = CharAreaType::Predefined;
	m_predefinedCharArea = area;
}
void VisualData::SetAdpativeCharArea()
{
	m_charAreaType = CharAreaType::Adaptive;
}

bool VisualData::HasPredefinedCharArea() const
{
	return m_charAreaType == CharAreaType::Predefined;
}
const Vec2& VisualData::GetPredefinedCharArea() const
{
	if (HasPredefinedCharArea()) return m_predefinedCharArea;
	return {};
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

std::string VisualData::ToString() const
{
	return std::format("[Visual CharSpacing:{} FontSize:{} Pivot:{} Text:{}]",
		m_charSpacing.ToString(), std::to_string(GetFontSize()), m_pivotRelative.GetPos().ToString(), m_Text.ToString());
}
