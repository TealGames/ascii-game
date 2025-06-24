#include "pch.hpp"
#include "Core/Visual/VisualDataParser.hpp"
#include "Core/Asset/GlobalColorCodes.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Utils/RaylibUtils.hpp"

static constexpr char NEW_ROW_CHAR = '-';
static constexpr char CHAR_COLOR_ALIAS_START = '[';
static const std::string CHAR_COLOR_ALIAS_END = "]";

VisualData ParseDefaultVisualData(const std::vector<std::string>& lines)
{
	if (lines.empty()) return {};

	Color currentColor = WHITE;
	std::vector<std::vector<TextChar>> textCharPos = {};
	FontProperties fontSettings = FontProperties(VisualData::DEFAULT_FONT_SIZE, 0, GetGlobalFont());
	for (const auto& line : lines)
	{
		for (size_t i = 0; i < line.size(); i++)
		{
			//LogError(std::format("Iteraing on proeprty:{} at:{} value:{} char:{}", figProperties[i].GetKey(), std::to_string(i), value, Utils::ToString(value[j])));
			if (i==0)
			{
				//arrPos.IncrementRow(1);
				//arrPos.SetCol(0);
				textCharPos.push_back(std::vector<TextChar>{});
				//continue;
			}

			if (line[i] == CHAR_COLOR_ALIAS_START && i < line.size() - CHAR_COLOR_ALIAS_END.size() - 1)
			{
				size_t colorAliasEndIndex = line.find(CHAR_COLOR_ALIAS_END, i + 1);
				if (!Assert(colorAliasEndIndex != std::string::npos, std::format("Tried to parse a color alias for visual data line: {} "
					"but did not find color alias end at color alias start at index: {}",
					line, std::to_string(i)))) 
					continue;

				const std::string colorCode = line.substr(i + 1, colorAliasEndIndex - (i + 1));
				std::optional<Color> maybeColor = GlobalColorCodes::TryGetColorFromCode(colorCode);
				//Only if we do have found a color do we set the new color
				if (Assert(maybeColor.has_value(), std::format("Tried to parse a color alias for visual data "
					"at line : {} but color code: {} starting at index:{} has no color data defined that can be found in global color codes",
					line, colorCode, std::to_string(i + 1))))
				{
					currentColor = maybeColor.value();
					//Assert(false, std::format("Found color:{}", RaylibUtils::ToString(maybeColor.value())));
				}

				i = colorAliasEndIndex;
				continue;
			}

			//TODO: change the color to use the color assigned in the animator
			textCharPos.back().push_back(TextChar(currentColor, line[i]));
			//arrPos.IncrementCol(1);
		}
	}
	//Assert(false, std::format("Parsed visual data"));
	return VisualData(textCharPos, GLOBAL_CHAR_SPACING, fontSettings, VisualData::DEFAULT_PIVOT);
}

VisualData ParseDefaultVisualData(const FigProperty& property)
{
	return ParseDefaultVisualData(property.m_Value);
}
