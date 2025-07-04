#include "pch.hpp"
#include "StaticReferenceGlobals.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/FontAsset.hpp"
#include "Utils/RaylibUtils.hpp"

static const char* DEFAULT_FONT_PATH = "fonts/default.ttf";
static AssetManagement::AssetManager* AssetManager;
static const char* RAYLIB_DEFAULT_FONT_ASSET_NAME = "RaylibDefaultFont";

namespace StaticReferenceGlobals
{
	void Init(AssetManagement::AssetManager& assetManager)
	{
		AssetManager = &assetManager;
		AssetManager->CreateRuntimeAsset<FontAsset>(RAYLIB_DEFAULT_FONT_ASSET_NAME, FontAsset(GetFontDefault()));
	}

	const FontAsset& StaticReferenceGlobals::GetGlobalFont()
	{
		FontAsset* font= AssetManager->TryGetTypeAssetFromPathMutable<FontAsset>(DEFAULT_FONT_PATH);
		if (font == nullptr)
		{
			Assert(false, std::format("Tried to get the default font from globals, "
				"but font asset is null. This means InitGlobals was not called"));
		}
		return* font;
	}
	const FontAsset& GetDefaultRaylibFont()
	{
		return *(AssetManager->TryGetRuntimeTypeAssetMutable<FontAsset>(RAYLIB_DEFAULT_FONT_ASSET_NAME));
	}
}


