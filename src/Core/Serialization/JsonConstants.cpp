#include "pch.hpp"
#include "Core/Serialization/JsonConstants.hpp"
#include <unordered_map>
#include "StaticGlobals.hpp"
#include "Core/Visual/VisualData.hpp"
#include "Utils/Data/ColorConstants.hpp"

namespace JsonConstants
{
	static const std::unordered_map<std::string, Utils::Color> COLOR_CONSTANTS = { {"White", Utils::COLOR_WHITE} };
	static const std::unordered_map<std::string, Vec2> PIVOT_CONSTANTS = { {"BottomCenter",VisualData::PIVOT_BOTTOM_CENTER}, {"BottomLeft",VisualData::PIVOT_BOTTOM_LEFT}, 
																	 {"BottomRight",VisualData::PIVOT_BOTTOM_RIGHT},
																	 {"Center",VisualData::PIVOT_CENTER}, {"TopCenter",VisualData::PIVOT_TOP_CENTER}, 
																	 {"TopLeft",VisualData::PIVOT_TOP_LEFT}, {"TopRight",VisualData::PIVOT_TOP_RIGHT}};

	/*static const std::unordered_map<std::string, Vec2> DIR_CONSTANTS = { {"N", Vec2::NORTH}, { "NE", Vec2::NORTHEAST}, { "E", Vec2::EAST}, { "SE", Vec2::SOUTHEAST}, 
																   { "S", Vec2::SOUTH}, {"SW", Vec2::SOUTHWEST}, {"W", Vec2::WEST}, {"NW", Vec2::NORTHWEST}};*/

	//static const std::unordered_map<std::string, Font> FONT_CONSTANTS = { {"Default", GetFontDefault()}};
	//Note: we cant just store a reference to font because this occurs on init before raylib gets set up
	static const std::string DEFAULT_FONT_NAME = "Default";
	static const std::unordered_map<std::string, Vec2> FONT_SIZE_CONSTANTS = { {"Default", GLOBAL_FONT_SIZE}};

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

	/*std::optional<Vec2> TryGetConstantDir(const std::string& constant)
	{
		return TryGetConstantValue<Vec2>(DIR_CONSTANTS, constant);
	}
	std::optional<std::string> TryGetDirConstant(const Vec2& vec)
	{
		return TryGetValueConstant<Vec2>(DIR_CONSTANTS, vec,
			[](const Vec2& vec1, const Vec2& vec2)-> bool { return vec1 == vec2; });
	}*/

	std::optional<Utils::Color> TryGetConstantColor(const std::string& constant)
	{
		return TryGetConstantValue<Utils::Color>(COLOR_CONSTANTS, constant);
	}

	std::optional<std::string> TryGetColorConstant(const Utils::Color& targetColor)
	{
		return TryGetValueConstant<Utils::Color>(COLOR_CONSTANTS, targetColor, 
			[](const Utils::Color& color1, const Utils::Color& color2)-> bool { return color1==color2; });
	}

	std::optional<Rendering::Font> TryGetConstantFont(const std::string& constant)
	{
		//TODO: add suppprt for default fonts
		//if (constant == DEFAULT_FONT_NAME) return GetFontDefault();
		return std::nullopt;
		//return TryGetConstantValue<Font>(FONT_CONSTANTS, constant);
	}
	std::optional<std::string> TryGetFontConstant(const Rendering::Font& font)
	{
		//TODO: implement
		return std::nullopt;
		/*if (RaylibUtils::FontEqual(font, GetFontDefault())) return DEFAULT_FONT_NAME;
		return std::nullopt;*/
		/*return TryGetValueConstant<Font>(FONT_CONSTANTS, font,
			[](const Font& font1, const Font& font2)-> bool { return RaylibUtils::FontEqual(font1, font2); });*/
	}

	std::optional<Vec2> TryGetConstantFontSize(const std::string& constant)
	{
		return TryGetConstantValue<Vec2>(FONT_SIZE_CONSTANTS, constant);
	}
	std::optional<std::string> TryGetFontSizeConstant(const Vec2& size)
	{
		return TryGetValueConstant<Vec2>(FONT_SIZE_CONSTANTS, size,
			[](const Vec2& size1, const Vec2& size2)-> bool { return size1 == size2; });
	}
}
