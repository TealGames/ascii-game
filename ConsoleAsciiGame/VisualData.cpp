#include "pch.hpp"
#include "VisualData.hpp"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"
#include "raylib.h"

const Utils::Point2D VisualData::PIVOT_TOP_LEFT = {0, 1};
const Utils::Point2D VisualData::PIVOT_TOP_RIGHT = {1, 1};
const Utils::Point2D VisualData::PIVOT_BOTTOM_LEFT = {0, 0};
const Utils::Point2D VisualData::PIVOT_BOTTOM_RIGHT = { 1, 0 };
const Utils::Point2D VisualData::PIVOT_CENTER = {0.5, 0.5};
const Utils::Point2D VisualData::PIVOT_BOTTOM_CENTER = {0.5, 0};
const Utils::Point2D VisualData::PIVOT_TOP_CENTER = {0.5, 1};

VisualData::VisualData() : VisualData(RawTextBufferBlock(), GetGlobalFont(), DEFAULT_FONT_SIZE, DEFAULT_CHAR_SPACING) {}
VisualData::VisualData(const RawTextBufferBlock& rawBuffer, const Font& font, 
	const std::uint8_t& fontSize, const Utils::Point2DInt& charSpacing, const Utils::Point2D& relativePivotPos)
	: m_Text(), m_font(&font), m_fontSize(fontSize), m_charSpacing(charSpacing), m_pivotRelative(relativePivotPos)
{
	if (rawBuffer.empty()) return;

	std::optional<TextArray> squaredBuffer = CreateSquaredBuffer(rawBuffer);
	if (!Utils::Assert(squaredBuffer.has_value(), std::format("Tried to create a Visual data with raw buffer: {}, "
		"but failed to square data", ToStringRawBuffer(rawBuffer)))) return;
	//Utils::Log(std::format("Square buffer val in optional: {}", squaredBuffer.value().ToString()));

	m_Text = squaredBuffer.value();
	ValidatePivot(m_pivotRelative);
}

bool VisualData::HasValidFont() const
{
	return Utils::Assert(this, RaylibUtils::IsValidFont(GetFont()), std::format("Tried to retrieve font for visual data: {} "
		"but the font is not valid (could be due to not being loaded properly)", m_Text.ToString()));
}

bool VisualData::ValidatePivot(Utils::Point2D& pivot) const
{
	if (!Utils::Assert(this, 0 <= pivot.m_X && pivot.m_X <=1 && 
		0 <= pivot.m_Y && pivot.m_Y <= 1, std::format("Validated pivot for visual: {} "
		"but it is not relative to visual so it was automatically adjusted", m_Text.ToString())))
	{
		pivot.m_X = std::clamp(pivot.m_X, (float)0, (float)1);
		pivot.m_Y = std::clamp(pivot.m_Y, (float)0, (float)1);
		return false;
	}
	return true;
}

////TODO: this function handles the scale in the final camera viewport which makes no sense
////it should intead just 
//void VisualData::TryUpdateTexture()
//{
//	if (!m_doesTextureNeedUpdating) return;
//
//	//TOOD: we do not set this to true when we update the text since that is a puplic member
//
//	Utils::Log("Trying to create texture");
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
	//Utils::Log(std::format("After intercolumn check max len: {} actual chars; {}", std::to_string(maxRowCharLen), TextBuffer::ToString(filledSpaces)));

	for (int r = 0; r < filledSpaces.size(); r++)
	{
		if (filledSpaces[r].size() == maxRowCharLen) continue;
		/*if (!Utils::Assert(filledSpaces[r].size() < maxRowCharLen,
			std::format("Tried to get rectangalized scene text for render layer but the current "
				"row size: {} is bigger than the max: {}",
				std::to_string(filledSpaces[r].size()), std::to_string(maxRowCharLen))))
			return std::nullopt;*/

		for (int i = 0; i < filledSpaces[r].size() - maxRowCharLen; i++)
			filledSpaces[r].push_back(TextChar(Color(), EMPTY_CHAR_PLACEHOLDER));
	}
	TextArray result = TextArray(filledSpaces[0].size(), filledSpaces.size(), filledSpaces);
	/*Utils::Log(std::format("Size: {}x{} result: {}",
		std::to_string(filledSpaces.size()), std::to_string(filledSpaces[0].size()), result.ToString()));*/
	return result;
}

Utils::Point2DInt VisualData::GetWorldSize() const
{
	if (!HasValidFont()) return {};

	int maxWidth = 0;
	int currentWidth = 0;
	int textureHeight = 0;
	Vector2 currentRowTextSize = {};
	std::string currentRowText = "";
	for (int r = 0; r < m_Text.GetHeight(); r++)
	{
		currentRowText = m_Text.GetStringAt(r);
		if (currentRowText.size() == 1) currentRowText += " ";

		currentRowTextSize= MeasureTextEx(GetFont(), currentRowText.c_str(), m_fontSize, 0);
		currentWidth = currentRowTextSize.x + (m_Text.GetWidth() - 1) * m_charSpacing.m_X;
		Utils::Log(std::format("Current width for: {} at: {} ({}) is: {} text sixe: {}", 
			m_Text.ToString(), std::to_string(r), currentRowText, std::to_string(currentWidth), std::to_string(currentRowTextSize.x)));
		if (currentWidth > maxWidth) maxWidth = currentWidth;

		textureHeight += currentRowTextSize.y;
		if (r != 0) textureHeight += m_charSpacing.m_Y;
	}
	Utils::Log(std::format("World size for: {} is: {}", m_Text.ToString(), Utils::Point2DInt{ maxWidth, textureHeight }.ToString()));
	return { maxWidth, textureHeight };
}

void VisualData::AddTextPositionsToBuffer(const WorldPosition& transformPos, TextBufferMixed& buffer) const
{
	if (!HasValidFont()) return;

	Utils::Point2D totalSize = {};
	float currentColsWidth = 0;
	float previousRowsHeight = 0;
	float currentRowMaxHeight = 0;
	Vector2 currentSize = {};
	

	char currentTextChar[2] = {'1', '\0'};
	

	std::vector<TextBufferPosition> textBufferData = {};

	//First pass will calculate distance from top left corner of the text using relative coords
	Utils::Point2DInt charArrSize = m_Text.GetSize();
	for (int r = 0; r < charArrSize.m_Y; r++)
	{
		currentRowMaxHeight = 0;
		currentColsWidth = 0;

		for (int c = 0; c < charArrSize.m_X; c++)
		{
			currentTextChar[0] = m_Text.GetAtUnsafe({ r, c }).m_Char;
			currentSize = MeasureTextEx(GetFont(), currentTextChar, m_fontSize, 0);
			if (currentSize.y > currentRowMaxHeight) currentRowMaxHeight = currentSize.y;

			if (c == 0)
			{
				textBufferData.push_back(TextBufferPosition{ Utils::Point2D{ 0, -previousRowsHeight },
				m_Text.GetAtUnsafe({r, c}), *m_font, m_fontSize });
			}

			//We do this here even for when c == width-1 so we can use be added towards total
			currentColsWidth += (currentSize.x + m_charSpacing.m_X);

			if (c < charArrSize.m_X - 1 && charArrSize.m_X >= 2)
			{
				//Note: we get plus one for col since positions is top left by calculating all previous width/height data
				textBufferData.push_back(TextBufferPosition{ Utils::Point2D{ currentColsWidth, -previousRowsHeight},
				m_Text.GetAtUnsafe({r, c + 1}), *m_font, m_fontSize });
			}
		}
		

		//Update the current total size based on the stored previous row and col sizes
		previousRowsHeight += (currentRowMaxHeight + m_charSpacing.m_Y);
		totalSize.m_Y = previousRowsHeight;
		if (currentColsWidth > totalSize.m_X) totalSize.m_X = currentColsWidth;
	}
	//Utils::Log(std::format("World size for: {} is: {}", m_Text.ToString(), totalSize.ToString()));

	WorldPosition topLeftPos = {transformPos.m_X - m_pivotRelative.m_X*totalSize.m_X, 
							    transformPos.m_Y+ (1- m_pivotRelative.m_Y)* totalSize.m_X };

	//Second pass will add the top left position calculated based on total size from first pass
	for (auto& textBuffer : textBufferData)
	{
		textBuffer.m_Pos.m_X += topLeftPos.m_X;
		textBuffer.m_Pos.m_Y += topLeftPos.m_Y;
		/*Utils::Log(std::format("Visual TRANSFORM POS: {} total size: {} top left:{} has buffer: {} ", 
			transformPos.ToString(), totalSize.ToString(), topLeftPos.ToString(), textBuffer.ToString()));*/
	}

	for (const auto& textBuffer : textBufferData)
	{
		buffer.emplace_back(textBuffer);
	}

	return;
}

const Utils::Point2DInt& VisualData::GetCharSpacing() const
{
	return m_charSpacing;
}

const Font& VisualData::GetFont() const
{
	if (!Utils::Assert(this, m_font != nullptr,
		std::format("Tried to get font for visual but the font is NULL")))
	{
		throw new std::invalid_argument("Invalid font member on VisualData");
	}
		
	return *m_font;
}
const std::uint8_t VisualData::GetFontSize() const
{
	return m_fontSize;
}

const Utils::Point2D& VisualData::GetPivot() const
{
	return m_pivotRelative;
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
