#include "pch.hpp"
#include "JsonConstants.hpp"
#include <unordered_map>
#include "RaylibUtils.hpp"
#include "Globals.hpp"
#include "VisualData.hpp"

namespace JsonConstants
{
	static const std::unordered_map<std::string, Color> COLOR_CONSTANTS = { {"White", WHITE} };
	static const std::unordered_map<std::string, Vec2> PIVOT_CONSTANTS = { {"BottomCenter",VisualData::PIVOT_BOTTOM_CENTER}, {"BottomLeft",VisualData::PIVOT_BOTTOM_LEFT}, {"BottomRight",VisualData::PIVOT_BOTTOM_RIGHT},
																	 {"Center",VisualData::PIVOT_CENTER}, {"TopCenter",VisualData::PIVOT_TOP_CENTER}, 
																	 {"TopLeft",VisualData::PIVOT_TOP_LEFT}, {"TopRight",VisualData::PIVOT_TOP_RIGHT}};
	static const std::unordered_map<std::string, Vec2> DIR_CONSTANTS = { {"N", Vec2::NORTH}, { "NE", Vec2::NORTHEAST}, { "E", Vec2::EAST}, { "SE", Vec2::SOUTHEAST}, 
																   { "S", Vec2::SOUTH}, {"SW", Vec2::SOUTHWEST}, {"W", Vec2::WEST}, {"NW", Vec2::NORTHWEST}};
	static const std::unordered_map<std::string, Font> FONT_CONSTANTS = { {"Default", GetFontDefault()}};
	static const std::unordered_map<std::string, float> FONT_SIZE_CONSTANTS = { {"Default", GLOBAL_FONT_SIZE}};

	template<typename Value>
	static std::optional<Value> TryGetConstantValue(const std::unordered_map<std::string, Value>& constants, const std::string& constant)
	{
		auto it = constants.find(constant);
		if (it == constants.end())
			return std::nullopt;

		return it->second;
	}

	template<typename Value>
	using ValueEqualityPredicate = std::function<bool(const Value& val1, const Value& val2)>;

	template<typename Value>
	static std::optional<std::string> TryGetValueConstant(const std::unordered_map<std::string, Value>& constants, const Value& target, 
		const ValueEqualityPredicate<Value>& equalityPredicate)
	{
		for (const auto& pair : constants)
		{
			if (equalityPredicate(pair.second, target))
				return pair.first;
		}
		return std::nullopt;
	}

	std::optional<Vec2> TryGetConstantPivot(const std::string& constant)
	{
		return TryGetConstantValue<Vec2>(PIVOT_CONSTANTS, constant);
	}
	std::optional<std::string> TryGetPivotConstant(const Vec2& vec)
	{
		return TryGetValueConstant<Vec2>(PIVOT_CONSTANTS, vec,
			[](const Vec2& vec1, const Vec2& vec2)-> bool { return vec1 == vec2; });
	}

	std::optional<Vec2> TryGetConstantDir(const std::string& constant)
	{
		return TryGetConstantValue<Vec2>(DIR_CONSTANTS, constant);
	}
	std::optional<std::string> TryGetDirConstant(const Vec2& vec)
	{
		return TryGetValueConstant<Vec2>(DIR_CONSTANTS, vec,
			[](const Vec2& vec1, const Vec2& vec2)-> bool { return vec1 == vec2; });
	}

	std::optional<Color> TryGetConstantColor(const std::string& constant)
	{
		return TryGetConstantValue<Color>(COLOR_CONSTANTS, constant);
	}

	std::optional<std::string> TryGetColorConstant(const Color& targetColor)
	{
		return TryGetValueConstant<Color>(COLOR_CONSTANTS, targetColor, 
			[](const Color& color1, const Color& color2)-> bool { return RaylibUtils::ColorEqual(color1, color2); });
	}

	std::optional<Font> TryGetConstantFont(const std::string& constant)
	{
		return TryGetConstantValue<Font>(FONT_CONSTANTS, constant);
	}
	std::optional<std::string> TryGetFontConstant(const Font& font)
	{
		return TryGetValueConstant<Font>(FONT_CONSTANTS, font,
			[](const Font& font1, const Font& font2)-> bool { return RaylibUtils::FontEqual(font1, font2); });
	}

	std::optional<float> TryGetConstantFontSize(const std::string& constant)
	{
		return TryGetConstantValue<float>(FONT_SIZE_CONSTANTS, constant);
	}
	std::optional<std::string> TryGetFontSizeConstant(const float& size)
	{
		return TryGetValueConstant<float>(FONT_SIZE_CONSTANTS, size,
			[](const float& size1, const float& size2)-> bool { return size1 == size2; });
	}
}
