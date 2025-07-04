#pragma once

namespace AssetManagement { class AssetManager; }
class FontAsset;

namespace StaticReferenceGlobals
{
	void Init(AssetManagement::AssetManager& assetManager);

	const FontAsset& GetGlobalFont();
	const FontAsset& GetDefaultRaylibFont();
}


