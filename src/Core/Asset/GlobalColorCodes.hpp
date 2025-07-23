#pragma once
//#include "raylib.h"
#include <optional>
#include "Fig/Fig.hpp"
#include "Utils/Data/Color.hpp"

namespace AssetManagement
{
	class AssetManager;
};

namespace GlobalColorCodes
{
	void InitCodes(const AssetManagement::AssetManager& assetManager);
	bool TryRegisterColor(const FigProperty& property);

	bool HasColor(const std::string& colorName);
	std::optional<Utils::Color> TryGetColorFromCode(const std::string& code);
}

