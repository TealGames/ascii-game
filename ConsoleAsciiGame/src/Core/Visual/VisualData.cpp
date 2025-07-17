#include "pch.hpp"
#include "Core/Visual/VisualData.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Utils/RaylibUtils.hpp"
#include "raylib.h"
#include "Core/Analyzation/Debug.hpp"

const Vec2& VisualData::DEFAULT_FONT_SIZE = GLOBAL_FONT_SIZE;

const Vec2 VisualData::PIVOT_TOP_LEFT = {0, 1};
const Vec2 VisualData::PIVOT_TOP_RIGHT = {1, 1};
const Vec2 VisualData::PIVOT_BOTTOM_LEFT = {0, 0};
const Vec2 VisualData::PIVOT_BOTTOM_RIGHT = { 1, 0 };
const Vec2 VisualData::PIVOT_CENTER = {0.5, 0.5};
const Vec2 VisualData::PIVOT_BOTTOM_CENTER = {0.5, 0};
const Vec2 VisualData::PIVOT_TOP_CENTER = {0.5, 1};

const Vec2 VisualData::DEFAULT_PIVOT = PIVOT_CENTER;

VisualDataPreset::VisualDataPreset(const FontAsset& font, const float& fontSize, const Vec2& charSpacing,
	const CharAreaType& charAreaType, const Vec2& predefinedCharArea, const NormalizedPosition& relativePivotPos) :
	m_FontAsset(&font), m_FontSize(fontSize), m_CharSpacing(charSpacing), m_CharAreaType(charAreaType), 
		m_PredefinedCharArea(predefinedCharArea), m_RelativePivotPos(relativePivotPos) {}

VisualData::VisualData() : VisualData({}, Vec2(), NormalizedPosition()) {}


VisualData::VisualData(const FragmentedTextBuffer& rawBuffer, const NormalizedPosition& relativePivotPos) :
	m_buffer(rawBuffer), m_pivotRelative(relativePivotPos), m_worldSize() {}

VisualData::VisualData(const std::vector<std::vector<TextBufferChar>>& rawBuffer, const Vec2& charSpacing,
	const NormalizedPosition& relativePivotPos) : 
	m_buffer(), m_pivotRelative(relativePivotPos), m_worldSize()
{
	CreateBuffer(rawBuffer, charSpacing);
}

//TODO: this needs to be cut down and abstracted to not have repetitive constructors
VisualData::VisualData(const std::vector<std::vector<TextChar>>& rawBuffer, const Vec2& charSpacing,
	const WorldFontProperties& fontSettings, const NormalizedPosition& relativePivotPos) : m_buffer(), m_pivotRelative(relativePivotPos), m_worldSize()
{
	std::vector<std::vector<TextBufferChar>> fontBuffer = {};
	for (const auto& textRow : rawBuffer)
	{
		fontBuffer.push_back({});
		for (const auto& text : textRow)
		{
			fontBuffer.back().push_back(TextBufferChar(text, fontSettings));
		}
	}
	CreateBuffer(fontBuffer, charSpacing);
}

VisualData::VisualData(const std::vector<std::vector<TextChar>>& rawBuffer, const Vec2& charArea, const Vec2& charSpacing,
	const WorldFontProperties& fontSettings, const NormalizedPosition& relativePivotPos) : m_buffer(), m_pivotRelative(relativePivotPos), m_worldSize()
{
	if (rawBuffer.empty()) return;
	
	size_t maxRowSize = rawBuffer[0].size();
	for (size_t r = 1; r < rawBuffer.size(); r++)
	{
		if (rawBuffer[r].size() > maxRowSize) maxRowSize = rawBuffer[r].size();
	}
	m_worldSize = {charArea.m_X * maxRowSize + (charSpacing.m_X * (maxRowSize-1)),
						   charArea.m_Y* rawBuffer.size() + (charSpacing.m_Y * (rawBuffer.size() - 1))};

	Vec2 pivotDiff = {};
	NormalizedPosition currPosNormalized = NormalizedPosition::TOP_LEFT;
	for (size_t r = 0; r < rawBuffer.size(); r++)
	{
		for (size_t c = 0; c < rawBuffer[r].size(); c++)
		{
			AddToCreatedBuffer(r, c, rawBuffer[r].size(), rawBuffer[r][c], fontSettings, charSpacing,
				pivotDiff, currPosNormalized, m_worldSize, nullptr, &charArea);
		}
	}
}

void VisualData::CreateBuffer(const std::vector<std::vector<TextBufferChar>>& rawBuffer, 
	const Vec2& charSpacing)
{
	if (rawBuffer.empty()) return;

	int maxRowChars = 0;
	float rowWidthSize = 0;
	float rowHeightSizeMax = 0;
	Vec2 unpaddedSize = {};
	Vec2 dataCharSize = {};
	for (const auto& row : rawBuffer)
	{
		rowWidthSize = 0;
		rowHeightSizeMax = 0;
		for (const auto& dataChar : row)
		{
			dataCharSize = dataChar.GetWorldSize();
			//LogError(std::format("Found char:{} size:{}", Utils::ToString(dataChar.m_Char), dataCharSize.ToString()));
			rowWidthSize += dataCharSize.m_X;
			if (dataCharSize.m_Y > rowHeightSizeMax)
				rowHeightSizeMax = dataCharSize.m_Y;
		}
		if (row.size() > maxRowChars)
			maxRowChars = row.size();

		if (rowWidthSize > unpaddedSize.m_X)
			unpaddedSize.m_X = rowWidthSize;
		unpaddedSize.m_Y += rowHeightSizeMax;

		//LogError(std::format("Row size:{} unpadded row:{}", std::to_string(rowWidthSize), unpaddedSize.ToString()));
	}

	NormalizedPosition currPosNormalized = NormalizedPosition::TOP_LEFT;
	float currentRowMaxHeight = 0;
	m_worldSize = { (maxRowChars - 1) * charSpacing.m_X + unpaddedSize.m_X,
								 (rawBuffer.size() - 1) * charSpacing.m_Y + unpaddedSize.m_Y };

	if (!Assert(m_worldSize.m_X != 0 && m_worldSize.m_Y != 0, std::format("Tried to create visual data but "
		"full size was calculated to invalid value:{} unpadded:{}", m_worldSize.ToString(), unpaddedSize.ToString())))
		return;

	Vec2 pivotDiff = {};
	for (size_t r = 0; r < rawBuffer.size(); r++)
	{
		for (size_t c = 0; c < rawBuffer[r].size(); c++)
		{
			AddToCreatedBuffer(r, c, rawBuffer[r].size(), rawBuffer[r][c].m_Text, rawBuffer[r][c].m_FontData, charSpacing,
				pivotDiff, currPosNormalized, m_worldSize, &currentRowMaxHeight, nullptr);
		}
	}
}

void VisualData::AddToCreatedBuffer(const size_t& r, const size_t& c, const size_t currRowElementCount, const TextChar& textChar, const WorldFontProperties& fontData, const Vec2& charSpacing,
	Vec2& pivotDiff, NormalizedPosition& currPosNormalized, const Vec2& fullSize, float* currentRowMaxHeight, const Vec2* predefinedCharArea)
{
	if (c == 0 && currentRowMaxHeight!=nullptr) *currentRowMaxHeight = 0;

	//If it is empty we still want to find the delta and increase it to cover its space
	//but we should not add it to the buffer
	Vec2 charSize = textChar.GetWorldSize(fontData);
	if (!textChar.IsEmpty())
	{
		//Note: we must subtract their positions and no pos norms directly because we do NOT want clamping from (0,0) to (1,1)
		//Note: we subtract curr pos - relative pos in order to get the correct order of RIGHT of pivot +, LEFT of pivot:- (UP +, DOWN -)
		pivotDiff = (currPosNormalized.GetPos() - m_pivotRelative.GetPos()) * fullSize;
		if (predefinedCharArea != nullptr)
		{
			//If the char size height is greater than predefined area, we just move it up by difference
			//if (charSize.m_Y > predefinedCharArea->m_Y) pivotDiff.m_Y += (charSize.m_Y - predefinedCharArea->m_Y);
			//If both x and y are smaller, we then center the char in the center of the area
			if (charSize.m_X < predefinedCharArea->m_X) pivotDiff += (*predefinedCharArea - charSize) / 2 * Vec2(1, -1);
		}

		m_buffer.push_back(TextBufferCharPosition(pivotDiff, textChar, fontData));
		/*LogError(std::format("Adding visual data char:{} current norm:{} pivot:{} pivots diff:{} size diff:{} ", m_buffer.back().ToString(),
			currPosNormalized.GetPos().ToString(), relativePivotPos.GetPos().ToString(), (relativePivotPos.GetPos() - currPosNormalized.GetPos()).ToString(), pivotDiff.ToString()));*/
	}
	if (currentRowMaxHeight!=nullptr && charSize.m_Y > *currentRowMaxHeight) *currentRowMaxHeight = charSize.m_Y;

	if (c == currRowElementCount - 1)
	{
		currPosNormalized.m_X = 0;
		currPosNormalized.m_Y -= ((currentRowMaxHeight != nullptr? *currentRowMaxHeight : predefinedCharArea->m_Y) + charSpacing.m_Y) / fullSize.m_Y;
	}
	else currPosNormalized.m_X += ((predefinedCharArea!=nullptr? predefinedCharArea->m_X : charSize.m_X) + charSpacing.m_X) / fullSize.m_X;
}

/*
void VisualData::AddTextPositionsToBuffer(const WorldPosition& globalTransformPos, FragmentedTextBuffer& buffer) const
{
	buffer.reserve(buffer.size()+ m_buffer.size());
	for (const auto& charPos : m_buffer)
	{
		if (!charPos.m_FontData.HasValidFont())
		{
			LogError(std::format("Attempted to add text positions to buffer "
				"but found invalid font on char pos:{}", charPos.ToString()));
				return;
		}
			
		buffer.push_back(charPos);
		buffer.back().m_Pos += globalTransformPos;
	}
}
*/

//TODO: this may be expensive so maybe make this a cached value
Vec2 VisualData::GetWorldSize() const
{
	return m_worldSize;

	//WorldPosition maxPos = {0, 0};
	//WorldPosition minPos = {std::numeric_limits<float>().max(), 
	//						std::numeric_limits<float>().max() };

	//WorldPosition charMin = {};
	//WorldPosition charMax = {};
	//Vec2 charSize = {};

	//for (const auto& textPos : m_buffer)
	//{
	//	charSize = textPos.GetScreenSize();

	//	charMin = textPos.m_Pos - Vec2(0, charSize.m_Y);
	//	if (charMin.m_X < minPos.m_X) minPos.m_X = charMin.m_X;
	//	if (charMin.m_Y < minPos.m_Y) minPos.m_Y = charMin.m_Y;

	//	charMax = textPos.m_Pos + Vec2(charSize.m_X, 0);
	//	if (charMax.m_X > maxPos.m_X) maxPos.m_X = charMax.m_X;
	//	if (charMax.m_Y > maxPos.m_Y) maxPos.m_Y = charMax.m_Y;
	//}
	////LogError(std::format("When calcualting size min:{} max:{}", minPos.ToString(), maxPos.ToString()));
	//return {std::abs(maxPos.m_X- minPos.m_X), std::abs(maxPos.m_Y- minPos.m_Y)};
}

const FragmentedTextBuffer& VisualData::GetBuffer() const
{
	return m_buffer;
}

bool VisualData::IsEmpty() const
{
	return m_buffer.empty();
}

//void VisualData::AddVisualLocationToBuffer(const size_t& r, const size_t& c, FragmentedTextBuffer& buffer) const
//{
//	const TextBufferCharPosition& charPos = buffer[r][c];
//	if (!charPos.m_FontData.HasValidFont())
//	{
//		LogError(std::format("Attempted to add text positions to buffer "
//			"but found invalid font on char pos:{}", charPos.ToString()));
//		return;
//	}
//
//	buffer.push_back(charPos);
//	buffer.back().m_Pos = mainCamera.ScreenToWorldPosition(charPos + )
//}

Vec2 VisualData::GetPivotRelative() const
{
	return m_pivotRelative.GetPos();
}
WorldPosition VisualData::GetPivotWorldPos(const WorldPosition& centerScreenPos) const
{	
	return (m_pivotRelative.GetPos() - PIVOT_CENTER) * GetWorldSize() + centerScreenPos;
}

std::string VisualData::ToString() const
{
	return std::format("[Visual Size:{} Pivot:{} Text:{}]",
		GetWorldSize().ToString(), m_pivotRelative.GetPos().ToString(), ::ToString(m_buffer));
}

/* 
--------------------------------------------------------------------
	TOOD: THE FOLLOWING SHOULD NOT BE DELETED SINCE IT MAY BE USEFL FOR FUTURE USE
--------------------------------------------------------------------

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
		if (maxWidth > 0) filledSpaces.back().reserve(maxWidth);

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
			if (c != 0 && !Assert(currArrPos.GetRow() == rawBuffer[r][c].m_RowColPos.GetRow(),
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
	for (r = 0; r < filledSpaces.size(); r++)
	{
		spaceDiff = maxWidth - filledSpaces[r].size()
		if (!Assert(spaceDiff >= 0, std::format("Tried to create rectangular buffer of raw buffer:{} "
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
}
*/