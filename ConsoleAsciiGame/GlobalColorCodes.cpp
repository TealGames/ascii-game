#include "pch.hpp"
#include "GlobalColorCodes.hpp"
#include <filesystem>
#include "AssetManager.hpp"

static const std::filesystem::path GLOBAL_COLOR_PATH = "global_colors.txt";
static std::unordered_map<std::string, Color> ColorCodes = {};

namespace GlobalColorCodes
{
	void InitCodes(const AssetManagement::AssetManager& assetManager)
	{
		assetManager.TryExecuteOnAssetFile(GLOBAL_COLOR_PATH, 
			[](const std::string* line)-> void 
			{
				if (line == nullptr) return;

				std::optional<FigProperty> maybeFig = Fig::TryGetPropertyFromLine(*line);
				if (!Assert(maybeFig.has_value(), std::format("Tried to add global colors "
					"from file but line : '{}' failed to be parsed into fig", *line)))
					return;

				if (!Assert(TryRegisterColor(maybeFig.value()), std::format("Tried to register "
					"global color with fig line:'{}' but failed", maybeFig.value().ToString())))
					return;
				//TryRegisterColor();
			});
	}

	bool TryRegisterColor(const FigProperty& property)
	{
		if (property.m_Value.size() != 1) return false;

		const std::uint8_t r = std::stoi(property.m_Value[0].substr(0, 2), nullptr, 16);
		const std::uint8_t g = std::stoi(property.m_Value[0].substr(2, 2), nullptr, 16);
		const std::uint8_t b = std::stoi(property.m_Value[0].substr(4, 2), nullptr, 16);
		std::uint8_t a = 255;
		if (property.m_Value[0].size() == 8)
			a = std::stoi(property.m_Value[0].substr(6, 2), nullptr, 16);

		Color color = { r, g, b, a };
		auto emplaceResult= ColorCodes.emplace(property.m_Key, color);
		return emplaceResult.second;
	}

	bool HasColor(const std::string& colorName)
	{
		return ColorCodes.find(colorName) != ColorCodes.end();
	}

	std::optional<Color> TryGetColorFromCode(const std::string& code)
	{
		auto colorCodeIt = ColorCodes.find(code);
		if (colorCodeIt == ColorCodes.end()) return std::nullopt;

		return colorCodeIt->second;
	}
}
