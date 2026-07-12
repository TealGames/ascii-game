#include "pch.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "Utils/Debug.hpp"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "Utils/HelperFunctions.hpp"
#include "Utils/ToStringFunctions.hpp"
#include "Utils/IOHandler.hpp"

namespace Engine::Rendering
{
	/// <summary>
	/// The default texture to file write quality used for JPG
	/// ranging from 1-100 where 100 is visually lossless
	/// </summary>
	static constexpr std::uint8_t JPG_WRITE_QUALITY = 80;

#define HDR_EXTENSIONS_MACRO ".hdr", ".exr"
	const std::array<std::string_view,2> TextureAsset::HDR_EXTENSIONS = { HDR_EXTENSIONS_MACRO };
	const std::array<std::string_view, 5> TextureAsset::EXTENSIONS = { ".png", ".jpg", ".jpeg", HDR_EXTENSIONS_MACRO };

	TextureAsset::TextureAsset(const std::filesystem::path& path)
		: Asset(path), m_texture()
	{
		ASSET_EXTENSION_CHECK

		WriteTextureFromFile(path, m_texture);
	}

	TextureAsset::~TextureAsset() {}

	bool TextureAsset::IsValidTexture() const
	{
		return m_texture.IsValid();
	}

	const Rendering::Texture& TextureAsset::GetTexture() const
	{
		if (!IsValidTexture())
		{
			Assert(false, "Attempted to get texture from asset:{} but is invalid", ToString());
			throw std::invalid_argument("Invalid texture state");
		}
		return m_texture;
	}
	Rendering::Texture& TextureAsset::GetTextureMutable()
	{
		if (!IsValidTexture())
		{
			Assert(false, "Attempted to get texture from asset:{} but is invalid", ToString());
			throw std::invalid_argument("Invalid texture state");
		}
		return m_texture;
	};

	void TextureAsset::UpdateAssetFromFile()
	{
		
	}

	bool HasTextureExtension(const std::filesystem::path& path)
	{
		return ::Utils::IO::DoesPathHaveExtension(path, TextureAsset::EXTENSIONS);
	}
	bool IsHdrTextureExtension(const std::filesystem::path& path)
	{
		return ::Utils::IO::DoesPathHaveExtension(path, TextureAsset::HDR_EXTENSIONS);
	}

	void WriteTextureFromFile(const std::filesystem::path& path, Rendering::Texture& tex)
	{
		//NOTE: most formats store TOP row of image at row 0, and BOTTOM row of image at (height - 1) row 
		//but OpenGL stores TOP row of image at (height - 1) row and BOTTOM row of image at row 0 in memory
		//so we must vertically flip
		//TODO: there should be a better way to do this that does not flip vertically by default because what 
		//happens if we have texture that is not OpenGL memory layout??
		stbi_set_flip_vertically_on_load(true);
		const bool isHdrTexture = IsHdrTextureExtension(path);

		int width = 0, height = 0, channels = 0;
		std::byte* data = nullptr;
		if (isHdrTexture)
		{
			data = reinterpret_cast<std::byte*>(stbi_loadf(path.string().c_str(), &width, &height, &channels, 0));
			//LogError(std::format("Hdr width:{} height:{} channelsL:{}", width, height, channels));
		}
		else
		{
			data = reinterpret_cast<std::byte*>(stbi_load(path.string().c_str(), &width, &height, &channels, 0));
			//LogError(std::format("Hdr width:{} height:{}", width, height));
		}

		if (data == nullptr)
		{
			LogError(std::format("Attempted to load texture from path:{} "
				"but failed. stbimage Error:{}", path.string(), stbi_failure_reason()));
			return;
		}

		Rendering::TexelStorageType internalStorage = Rendering::TexelStorageType::RGBA8;
		if (isHdrTexture)
		{
			//NOTE: stbi ALWAYS returns a float32 per channel for HDR
			if (channels == 3) internalStorage = Rendering::TexelStorageType::RGB32F;
			else if (channels == 4) internalStorage = Rendering::TexelStorageType::RGBA32F;
			else
			{
				LogError(std::format("Tried to get internal storage for hdr texture asset "
					"but it has unsupported channels:{}", channels));
				return;
			}
		}
		else if (channels == 1) internalStorage = Rendering::TexelStorageType::R8;
		else if (channels == 3) internalStorage = Rendering::TexelStorageType::RGB8;
		else if (channels == 4) internalStorage = Rendering::TexelStorageType::RGBA8;
		else
		{
			LogError(std::format("Attempted to load texture asset but "
				"received a color channel count:{} with no actions", channels));
			return;
		}
		tex = Rendering::CreateTexture(data, Vec2Int(width, height), Rendering::TextureBufferType::GPU, internalStorage);
		/*if (path.stem() == "skybox_stylized_night") LogError(std::format("Data non null:{} tex empty?:{} import empty:{}",
			data != nullptr, tex.HasEmptyData(), Rendering::IsEmptyTexture(data, width * height * channels)));*/

		stbi_image_free(data);
	}
	void WriteTextureToFile(const std::filesystem::path& path, const Rendering::Texture& tex)
	{
		//Same as writing from file we need to flip. Look at TODO note there for similar feedback here
		stbi_flip_vertically_on_write(true);

		const bool isHdrTexture = tex.IsHDRTexture();
		const std::string pathExtension = path.extension().string();
		const Rendering::TextureInfo& texInfo = tex.GetInfo();
		const std::uint8_t channelCount = Rendering::GetChannelCount(texInfo.m_InternalStorage);
		//TODO: it would probably be best to create our own writer/reader for textures so we can support float16
		const bool hasFloat16Channel = (Rendering::GetChannelDataTypeFromStorage(texInfo.m_InternalStorage)
			== Rendering::TexelChannelDataType::Float16);
		const std::byte* dataPtr = nullptr;
		std::byte* readHeapPtr = nullptr;
		//If we have CPU texture, we want to get the direct cpu pointer with NO COPY to save memory
		//otherwise, we will have to allocate heap space and read from GPU 
		//AND if the channels are stored in float16 since stbi doesn't allow this we must convert
		if (tex.HasCPUBuffer() && !hasFloat16Channel) dataPtr = tex.GetCPUMemPtr();
		//If we have float16, we must expand for each channel to be a float32
		//AND we must also override the read bytes to be float32 rather than its default
		else
		{
			const size_t totalByteSize = hasFloat16Channel ? tex.CalculateTotalByteSize(texInfo.m_TexelSize,
				Rendering::GetChannelFormatFromStorage(texInfo.m_InternalStorage), Rendering::TexelChannelDataType::Float32) : tex.CalculateTotalByteSize();

			readHeapPtr = new std::byte[totalByteSize];
			tex.ReadBytes(Vec2Int::Zero(), texInfo.m_TexelSize, readHeapPtr,
				hasFloat16Channel ? std::make_optional(Rendering::TexelChannelDataType::Float32) : std::nullopt);
			dataPtr = readHeapPtr;
		}

		if (isHdrTexture)
		{
			ENGINE_ASSERT(pathExtension == ".hdr", "Attempted to write HDR texture:{} to path:{} "
				"but it does not have only supported HDR extension '.hdr'", tex.ToString(), path.string());
			/*ENGINE_ASSERT(channelCount == 3, "Attempted to write HDR texture:{} to path:{} "
				"but .hdr extension only supports 3 color channel format: RGB", tex.ToString(), path.string());*/

			stbi_write_hdr(path.string().c_str(), texInfo.m_TexelSize.m_X, texInfo.m_TexelSize.m_Y,
				channelCount, reinterpret_cast<const float*>(dataPtr));
		}
		else if (pathExtension == ".png")
		{
			const size_t texelWidth = texInfo.m_TexelSize.m_X;
			stbi_write_png(path.string().c_str(), texelWidth, texInfo.m_TexelSize.m_Y, channelCount, dataPtr, tex.GetTexelByteSize() * texelWidth);
		}
		else if (pathExtension == ".jpg")
		{
			stbi_write_jpg(path.string().c_str(), texInfo.m_TexelSize.m_X, texInfo.m_TexelSize.m_Y, channelCount, dataPtr, JPG_WRITE_QUALITY);
		}
		else
		{
			LogError(std::format("Attempted to write texture:{} to path:{} but the extension:{} is not supported",
				tex.ToString(), path.string(), pathExtension));
		}

		if (readHeapPtr != nullptr)
			delete[] readHeapPtr;
	}

}

