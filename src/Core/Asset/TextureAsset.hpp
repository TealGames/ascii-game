#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Rendering/Texture.hpp"

namespace Engine::Rendering
{
	class TextureAsset : public Assets::Asset
	{
	private:
		Rendering::Texture m_texture;
	public:
		static const std::array<std::string_view, 5> EXTENSIONS;
		static const std::array<std::string_view, 2> HDR_EXTENSIONS;
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

	bool HasTextureExtension(const std::filesystem::path& path);
	bool IsHdrTextureExtension(const std::filesystem::path& path);

	void WriteTextureFromFile(const std::filesystem::path& path, Rendering::Texture& tex);
	void WriteTextureToFile(const std::filesystem::path& path, const Rendering::Texture& tex);
}

