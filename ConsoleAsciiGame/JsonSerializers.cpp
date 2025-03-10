#include "pch.hpp"
#include "JsonSerializers.hpp"
#include "JsonConstants.hpp"
#include "JsonUtils.hpp"
#include "Debug.hpp"

static bool HasRequiredProperties(const Json& json, const std::vector<std::string>& propertyNames)
{
	bool hasAllProperties = true;
	for (const auto& propertyName : propertyNames)
	{
		if (!Assert(json.contains(propertyName), std::format("Tried to parse json: '{}' to type "
			"but it is missing the property: '{}'", JsonUtils::ToStringProperties(json), propertyName)))
		{
			hasAllProperties= false;
		}
	}
	return hasAllProperties;
}

void from_json(const Json& json, Vec2& vec)
{
	vec.m_X = json.at("X").get<float>();
	vec.m_Y = json.at("Y").get<float>();
}
void to_json(Json& json, const Vec2& vec)
{
	json = { {"X", vec.m_X}, {"Y", vec.m_Y} };
}

void from_json(const Json& json, Vec2Int& vec)
{
	vec.m_X = json.at("X").get<int>();
	vec.m_Y = json.at("Y").get<int>();
}
void to_json(Json& json, const Vec2Int& vec)
{
	json = { {"X", vec.m_X}, {"Y", vec.m_Y} };
}

void from_json(const Json& json, Array2DPosition& pos)
{
	pos = Array2DPosition(json.at("Row").get<int>(), json.at("Col").get<int>());
}
void to_json(Json& json, const Array2DPosition& pos)
{
	json = { {"Row", pos.GetRow()}, {"Col", pos.GetCol()}};
}

void from_json(const Json& json, RenderLayerType& layer)
{
	layer = GetLayersFromStrings(json.get<std::vector<std::string>>());
}
void to_json(Json& json, const RenderLayerType& layer)
{
	json = GetLayersAsStrings(layer);
}

void from_json(const Json& json, Color& color)
{
	if (!HasRequiredProperties(json, { "R", "G", "B" })) return;

	std::optional<std::string> maybeStringProperty = JsonUtils::TryGet<std::string>(json);
	if (maybeStringProperty.has_value())
	{
		std::optional<Color> maybeConstantColor = JsonConstants::TryGetConstantColor(maybeStringProperty.value());
		if (!Assert(maybeConstantColor.has_value(), std::format("Tried to convert json:'{} to color using constant "
			"names but it matches no constants!'", JsonUtils::ToStringProperties(json))))
			return;

		color = maybeConstantColor.value();
		return;
	}
	color.r = json.at("R").get<std::uint8_t>();
	color.g = json.at("G").get<std::uint8_t>();
	color.b = json.at("B").get<std::uint8_t>();

	if (json.contains("A")) color.a = json.at("A").get<std::uint8_t>();
	else color.a = 255;
}
void to_json(Json& json, const Color& color)
{
	std::optional<std::string> maybeConstant = JsonConstants::TryGetColorConstant(color);
	if (maybeConstant.has_value())
	{
		json = { maybeConstant.value() };
		return;
	}

	json= { {"R", color.r}, {"G", color.g}, {"B", color.b}, {"A", color.a}};
}

void from_json(const Json& json, TextChar& textChar)
{
	const char* COLOR_PROPERTY = "Color";
	const char* CHAR_PROPERTY = "Char";
	if (!HasRequiredProperties(json, { COLOR_PROPERTY, CHAR_PROPERTY })) return;

	textChar = TextChar(json.at(COLOR_PROPERTY).get<Color>(), json.at(CHAR_PROPERTY).get<char>());
}
void to_json(Json& json, const TextChar& textChar)
{
	json = { {"Color", textChar.m_Char}, {"Char", textChar.m_Char}};
}

void from_json(const Json& json, TextCharPosition& textChar)
{
	const char* COLOR_PROPERTY = "Color";
	const char* CHAR_PROPERTY = "Char";
	const char* POS_PROPERTY = "Pos";
	if (!HasRequiredProperties(json, { COLOR_PROPERTY, CHAR_PROPERTY, POS_PROPERTY })) return;

	textChar = TextCharPosition(json.at(POS_PROPERTY).get<Array2DPosition>(),
		TextChar(json.at(COLOR_PROPERTY).get<Color>(), json.at(CHAR_PROPERTY).get<std::string>()[0]));
}
void to_json(Json& json, const TextCharPosition& textChar)
{
	json = { {"Pos", textChar.m_RowColPos}, {"Char", textChar.m_Text.m_Char}, {"Color", textChar.m_Text.m_Color}};
}

void from_json(const Json& json, VisualData& visualData)
{
	const char* BUFFER_PROPERTY = "Buffer";
	const char* FONT_PROEPRTY = "Font";
	const char* FONT_SIZE_PROPERTY = "FontSize";
	const char* PIVOT_PROPERTY = "Pivot";
	const char* CHAR_SPACING_PROPERTY = "CharSpacing";
	const char* CHAR_AREA_PROPERTY = "CharArea";
	if (!HasRequiredProperties(json, { BUFFER_PROPERTY, FONT_PROEPRTY, FONT_SIZE_PROPERTY, 
		PIVOT_PROPERTY, CHAR_SPACING_PROPERTY })) return;

	auto textChars = json.at(BUFFER_PROPERTY).get<std::vector<std::vector<TextCharPosition>>>();

	std::optional<Font> maybeFont = JsonConstants::TryGetConstantFont(json.at(FONT_PROEPRTY).get<std::string>());
	if (!Assert(maybeFont.has_value(), std::format("Tried to convert json: {} to visual data but font "
		"could not be deduced from '{}' property", JsonUtils::ToStringProperties(json), FONT_PROEPRTY)))
		return;

	
	float fontSize = 0;
	Json fontJson = json.at(FONT_SIZE_PROPERTY);
	if (fontJson.is_string())
	{
		LogError("Reached font json string");
		std::optional<float> maybeFontSize = JsonConstants::TryGetConstantFontSize(fontJson.get<std::string>());
		if (!Assert(maybeFontSize.has_value(), std::format("Tried to convert json: {} to visual data but font "
			"size could not be deduced from '{}' property", JsonUtils::ToStringProperties(json), FONT_SIZE_PROPERTY)))
			return;
		fontSize = maybeFontSize.value();
	}
	else fontSize = fontJson.get<float>();

	Vec2 pivotPos = VisualData::DEFAULT_PIVOT;
	Json pivotJson = json.at(PIVOT_PROPERTY);
	if (fontJson.is_string())
	{
		std::optional<Vec2> maybePivot = JsonConstants::TryGetConstantPivot(pivotJson.get<std::string>());
		if (!Assert(maybePivot.has_value(), std::format("Tried to convert json: {} to visual data but pivot "
			"could not be deduced from '{}' property", JsonUtils::ToStringProperties(json), PIVOT_PROPERTY)))
			return;
		pivotPos = maybePivot.value();
	}
	else pivotPos = fontJson.get<Vec2>();

	visualData = VisualData(textChars, maybeFont.value(), fontSize, json.at(CHAR_SPACING_PROPERTY).get<Vec2>(), NormalizedPosition(pivotPos));
	if (json.contains(CHAR_AREA_PROPERTY))
	{
		visualData.SetPredefinedCharArea(json.at(CHAR_AREA_PROPERTY).get<Vec2>());
	}
}
void to_json(Json& json, const VisualData& visualData)
{
	std::optional<std::string> maybeFontConstant = JsonConstants::TryGetFontConstant(visualData.GetFont());
	if (!Assert(maybeFontConstant.has_value(), std::format("Tried to convert visual data: {} to json but font constant"
		"could not be deduced from font", visualData.ToString())))
		return;

	json.clear();
	json["Buffer"] = visualData.GetRawBuffer();
	json["Font"] = maybeFontConstant.value(); 

	std::optional<std::string> maybeFontSizeConstant = JsonConstants::TryGetFontSizeConstant(visualData.GetFontSize());
	if (maybeFontSizeConstant.has_value()) json["FontSize"] = maybeFontSizeConstant.value();
	else json["FontSize"] = visualData.GetFontSize();

	std::optional<std::string> maybePivotConstant = JsonConstants::TryGetPivotConstant(visualData.GetPivot());
	if (maybeFontSizeConstant.has_value()) json["Pivot"] = maybePivotConstant.value();
	else json["Pivot"] = visualData.GetPivot();

	json["CharSpacing"] = visualData.GetCharSpacing();
	if (visualData.HasPredefinedCharArea()) json["CharArea"] = visualData.GetPredefinedCharArea();
}