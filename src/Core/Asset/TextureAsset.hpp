#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Rendering/Texture.hpp"

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
bool IsHdrTextureExtension(const std::string& extension);

void WriteTextureFromFile(const std::filesystem::path& path, Rendering::Texture& tex);
void WriteTextureToFile(const std::filesystem::path& path, const Rendering::Texture& tex);

