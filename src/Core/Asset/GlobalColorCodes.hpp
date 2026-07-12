#pragma once
//#include "raylib.h"
#include <optional>
#include "Fig/Fig.hpp"
#include "Core/Primitives/Color.hpp"

namespace Engine::Assets { class AssetManager; }

namespace Engine::GlobalColorCodes
{
	using AssetManager = Engine::Assets::AssetManager;
	void InitCodes(const AssetManager& assetManager);
	bool TryRegisterColor(const FigFormat::FigProperty& property);

	bool HasColor(const std::string& colorName);
	std::optional<ColHDR4> TryGetColorFromCode(const std::string& code);
}

