#include "pch.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "Utils/Debug.hpp"
#include "stb/stb_image.h"

//static constexpr size_t DEFAULT_CHANNELS = 3;

TextureAsset::TextureAsset(const std::filesystem::path& path)
	: Asset(path, false), m_texture() 
{
	/*if (!Assert(path.extension() == EXTENSION, std::format("Tried to create a texture asset from path:{} (extension:{})"
		"but it does not have required extension:'{}'", path.string(), path.extension().string(), EXTENSION)))
		return;*/

	int width = 0, height = 0, channels = 0;
	std::byte* data = reinterpret_cast<std::byte*>(stbi_load(path.string().c_str(), &width, &height, &channels, 0));
	if (data == nullptr)
	{
		LogError(std::format("Attempted to load texture from path:{} "
			"but failed. stbimage Error:{}", path.string(), stbi_failure_reason()));
		return;
	}
	//for (size_t i = 0; i < width * height * 4; i++)
	//{
	//	//LogWarning(std::format("Reading pixel:{} channel:{} value:{}", i/4, i%5, data[i + 335*160*4 + 160*4]));
	//	if (data[i]==255 && i % 4 == 3) LogError(std::format("Frist non-255 alpha:{},{}", i/4%width, i/4/width));
	//}
	//LogError(std::format("SHIT channels:{}", channels));
	//LogError(std::format("path:{} Image chnnaels:{} wid:{} heigh:{}", path.string(), channels, width, height));
	
	Rendering::TexelStorageType internalStorage = Rendering::TexelStorageType::RGBA8;
	if (channels == 1) internalStorage = Rendering::TexelStorageType::R8;
	else if (channels == 3) internalStorage = Rendering::TexelStorageType::RGB8;
	else if (channels == 4) internalStorage = Rendering::TexelStorageType::RGBA8;
	else
	{
		LogError(std::format("Attempted to load texture asset but "
			"received a color channel count:{} with no actions", channels));
		return;
	}

	m_texture = Rendering::CreateTexture(data, Vec2Int(width, height), internalStorage);
	stbi_image_free(data);
}

TextureAsset::~TextureAsset()
{
	//UNLOAD TEXTURE
	//UnloadTexture(m_texture);
}

bool HasTextureExtension(const std::string& path)
{
	return path == ".png" || path == ".jpg";
}

bool TextureAsset::IsValidTexture() const
{
	return m_texture.IsValid();
}

const Rendering::Texture& TextureAsset::GetTexture() const
{
	if (!IsValidTexture())
	{
		Assert(false, std::format("Attempted to get texture from asset:{} but is invalid", ToString()));
		throw std::invalid_argument("Invalid texture state");
	}
	return m_texture;
}
Rendering::Texture& TextureAsset::GetTextureMutable()
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
	//TODO: load texture
	//m_texture = LoadTexture(GetPath().string().c_str());
}

