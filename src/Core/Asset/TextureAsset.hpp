#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Rendering/Texture.hpp"
//#include "raylib.h"

class TextureAsset : public Asset
{
private:
	Rendering::Texture m_texture;
public:

private:
public:
	TextureAsset(const std::filesystem::path& path);
	~TextureAsset();

	bool IsValidTexture() const;
	const Rendering::Texture& GetTexture() const;
	Rendering::Texture& GetTextureMutable();
	//VisualData& GetTextureMutable();
	
	void UpdateAssetFromFile() override;
};

bool HasTextureExtension(const std::string& extension);

