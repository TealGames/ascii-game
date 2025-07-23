#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Rendering/Font.hpp"
//#include "raylib.h"
#include <optional>

class FontAsset : public Asset
{
private:
	Rendering::Font m_font;

public:
	static const std::string EXTENSION;

private:
public:
	FontAsset(const std::filesystem::path& path);
	FontAsset(const Rendering::Font& font);
	~FontAsset();

	Rendering::Font& GetFontMutable();
	const Rendering::Font& GetFont() const;
	bool HasValidFont() const;

	void UpdateAssetFromFile() override;
};

