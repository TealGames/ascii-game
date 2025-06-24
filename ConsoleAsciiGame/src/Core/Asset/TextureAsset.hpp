#pragma once
#include "Core/Asset/Asset.hpp"
#include "raylib.h"

class TextureAsset : public Asset
{
private:
	Texture2D m_texture;
public:
	static const std::string EXTENSION;

private:
public:
	TextureAsset(const std::filesystem::path& path);
	~TextureAsset();

	bool IsValidTexture() const;
	const Texture2D& GetTexture() const;
	//VisualData& GetTextureMutable();

	void UpdateAssetFromFile() override;
};

