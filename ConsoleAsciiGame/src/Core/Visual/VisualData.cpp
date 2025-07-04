#include "pch.hpp"
#include "Core/Visual/VisualData.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Utils/RaylibUtils.hpp"
#include "raylib.h"
#include "Core/Analyzation/Debug.hpp"

const float& VisualData::DEFAULT_FONT_SIZE = GLOBAL_FONT_SIZE;

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
	m_buffer(rawBuffer), m_pivotRelative(relativePivotPos)
{
}

VisualData::VisualData(const std::vector<std::vector<TextBufferChar>>& rawBuffer, const Vec2& charSpacing,
	const NormalizedPosition& relativePivotPos) : 
	m_buffer(), m_pivotRelative(relativePivotPos)
{
	CreateBuffer(rawBuffer, charSpacing, relativePivotPos);
}

//TODO: this needs to be cut down and abstracted to not have repetitive constructors
VisualData::VisualData(const std::vector<std::vector<TextChar>>& rawBuffer, const Vec2& charSpacing,
	const FontProperties& fontSettings, const NormalizedPosition& relativePivotPos) : m_buffer(), m_pivotRelative(relativePivotPos)
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
	CreateBuffer(fontBuffer, charSpacing, relativePivotPos);
}

void VisualData::CreateBuffer(const std::vector<std::vector<TextBufferChar>>& rawBuffer, 
	const Vec2& charSpacing, const NormalizedPosition& relativePivotPos)
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
	Vec2 charSize = {};
	Vec2 fullSize = { (maxRowChars - 1) * charSpacing.m_X + unpaddedSize.m_X,
								 (rawBuffer.size() - 1) * charSpacing.m_Y + unpaddedSize.m_Y };

	if (!Assert(fullSize.m_X != 0 && fullSize.m_Y != 0, std::format("Tried to create visual data but "
		"full size was calculated to invalid value:{} unpadded:{}", fullSize.ToString(), unpaddedSize.ToString())))
		return;

	for (size_t r = 0; r < rawBuffer.size(); r++)
	{
		currentRowMaxHeight = 0;
		for (size_t c = 0; c < rawBuffer[r].size(); c++)
		{
			//If it is empty we still want to find the delta and increase it to cover its space
			//but we should not add it to the buffer
			if (!rawBuffer[r][c].m_Text.IsEmpty())
			{
				//Note: we must subtract their positions and no pos norms directly because we do NOT want clamping from (0,0) to (1,1)
				//Note: we subtract curr pos - relative pos in order to get the correct order of RIGHT of pivot +, LEFT of pivot:- (UP +, DOWN -)
				Vec2 pivotDiff = (currPosNormalized.GetPos() - relativePivotPos.GetPos()) * fullSize;

				m_buffer.push_back(TextBufferCharPosition(pivotDiff, rawBuffer[r][c].m_Text, rawBuffer[r][c].m_FontData));
				/*LogError(std::format("Adding visual data char:{} current norm:{} pivot:{} pivots diff:{} size diff:{} ", m_buffer.back().ToString(),
					currPosNormalized.GetPos().ToString(), relativePivotPos.GetPos().ToString(), (relativePivotPos.GetPos() - currPosNormalized.GetPos()).ToString(), pivotDiff.ToString()));*/
			}

			charSize = rawBuffer[r][c].GetWorldSize();
			if (charSize.m_Y > currentRowMaxHeight) currentRowMaxHeight = charSize.m_Y;

			//Note: pos normalized has normal x and y axes (think first quadrant of cartesian coordinates)
			//and size is always in row, col so they will need to be opposites when applying changes
			if (c == rawBuffer[r].size() - 1)
			{
				currPosNormalized.m_X = 0;
				currPosNormalized.m_Y -= ((currentRowMaxHeight + charSpacing.m_Y) / fullSize.m_Y);
			}
			else
			{
				currPosNormalized.m_X += ((charSize.m_X + charSpacing.m_X) / fullSize.m_X);
			}
		}
	}
	//if (m_buffer.size() > 15) Assert(false, std::format("Created visual data full size:{} spacing{} :{}", fullSize.ToString(), charSpacing.ToString(), ToString()));
}

void VisualData::AddTextPositionsToBuffer(const WorldPosition& globalTransformPos, FragmentedTextBuffer& buffer) const
{
	for (const auto& charPos : m_buffer)
	{
		if (!Assert(charPos.m_FontData.HasValidFont(), std::format("Attempted to add text positions to buffer "
			"but found invalid font on char pos:{}", charPos.ToString()))) 
			return;

		buffer.push_back(charPos);
		buffer.back().m_Pos = buffer.back().m_Pos+ globalTransformPos;
	}
	/*if (m_charAreaType == CharAreaType::Predefined) AddTextPositionsToBufferPredefined(transformPos, buffer);
	else if (m_charAreaType == CharAreaType::Adaptive) AddTextPositionsToBufferAdaptive(transformPos, buffer);
	else
	{
		LogError(std::format("Tried to add text positions to buffer at transform: {} "
			"but char area tyoe has no actions defiend", transformPos.ToString()));
	}*/
}

//TODO: this may be expensive so maybe make this a cached value
Vec2 VisualData::GetWorldSize() const
{
	WorldPosition maxPos = {0, 0};
	WorldPosition minPos = {std::numeric_limits<float>().max(), 
							std::numeric_limits<float>().max() };

	WorldPosition charMin = {};
	WorldPosition charMax = {};
	Vec2 charSize = {};

	for (const auto& textPos : m_buffer)
	{
		charSize = textPos.GetWorldSize();

		charMin = textPos.m_Pos - Vec2(0, charSize.m_Y);
		if (charMin.m_X < minPos.m_X) minPos.m_X = charMin.m_X;
		if (charMin.m_Y < minPos.m_Y) minPos.m_Y = charMin.m_Y;

		charMax = textPos.m_Pos + Vec2(charSize.m_X, 0);
		if (charMax.m_X > maxPos.m_X) maxPos.m_X = charMax.m_X;
		if (charMax.m_Y > maxPos.m_Y) maxPos.m_Y = charMax.m_Y;
	}
	//LogError(std::format("When calcualting size min:{} max:{}", minPos.ToString(), maxPos.ToString()));
	return {std::abs(maxPos.m_X- minPos.m_X), std::abs(maxPos.m_Y- minPos.m_Y)};
}

const FragmentedTextBuffer& VisualData::GetBuffer() const
{
	return m_buffer;
}

bool VisualData::IsEmpty() const
{
	return m_buffer.empty();
}


const Vec2& VisualData::GetPivot() const
{
	return m_pivotRelative.GetPos();
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