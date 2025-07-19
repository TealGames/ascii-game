#include "pch.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "Core/Analyzation/Debug.hpp"

const std::string TextureAsset::EXTENSION = ".png";

TextureAsset::TextureAsset(const std::filesystem::path& path)
	: Asset(path, false), m_texture(LoadTexture(path.string().c_str())) {}

TextureAsset::~TextureAsset()
{
	UnloadTexture(m_texture);
}

bool TextureAsset::IsValidTexture() const
{
	return m_texture.id != 0;
}

const Texture2D& TextureAsset::GetTexture() const
{
	if (!IsValidTexture())
	{
		Assert(false, std::format("Attempted to get texture from asset:{} but is invalid", ToString()));
		throw std::invalid_argument("Invalid texture state");
	}
	return m_texture;
}
//VisualData& GetTextureMutable();

void TextureAsset::UpdateAssetFromFile()
{
	m_texture = LoadTexture(GetPath().string().c_str());
}

