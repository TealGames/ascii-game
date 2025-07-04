#pragma once
#include "Core/Asset/Asset.hpp"
#include "raylib.h"
#include <optional>

class FontAsset : public Asset
{
private:
	Font m_font;

public:
	static const std::string EXTENSION;

private:
public:
	FontAsset(const std::filesystem::path& path);
	FontAsset(const Font& font);
	~FontAsset();

	Font& GetFontMutable();
	const Font& GetFont() const;
	bool HasValidFont() const;

	void UpdateAssetFromFile() override;
};

