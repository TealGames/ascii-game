#include "pch.hpp"
#include "AssetManager.hpp"
#include "Debug.hpp"
#include "SceneAsset.hpp"
#include "InputProfileAsset.hpp"
#include "SpriteAnimationAsset.hpp"
#include "SpriteAsset.hpp"
#include "FontAsset.hpp"
#include "TextureAsset.hpp"
#include "IOHandler.hpp"

static bool THROW_ON_UNKNWON_ASSET = false;

namespace AssetManagement
{
	std::filesystem::path AssetManager::ASSET_PATH = "assets";

	AssetManager::AssetManager() : m_assets(), m_allFiles() 
	{
		if (!Assert(this, std::filesystem::exists(ASSET_PATH), std::format("Tried to add all assets at path:{} "
			"but path is invalid", ASSET_PATH.string())))
			return;

		std::string fileExtension = "";
		for (const auto& file : std::filesystem::recursive_directory_iterator(ASSET_PATH))
		{
			if (!std::filesystem::is_regular_file(file)) continue;

			//TODO: identity asset type from extension now int is placeholder
			fileExtension = file.path().extension().string();

			//We add each file based on its extension to the list
			auto existingExtensionIt = m_allFiles.find(fileExtension);
			if (existingExtensionIt == m_allFiles.end())
			{
				m_allFiles.emplace(fileExtension, std::vector<std::filesystem::path>{file.path()});
			}
			else
			{
				existingExtensionIt->second.emplace_back(file.path());
			}

			//We try to convert this file into a useable asset 
			if (fileExtension == SceneAsset::EXTENSION)
			{
				if (!Assert(this, TryCreateAssetFromFile<SceneAsset>(file.path()),
					std::format("Tried to add asset at path:{} as scene asset but failed", file.path().string())))
					return;
			}
			else if (fileExtension == InputProfileAsset::EXTENSION)
			{
				if (!Assert(this, TryCreateAssetFromFile<InputProfileAsset>(file.path()),
					std::format("Tried to add asset at path:{} as input profile asset but failed", file.path().string())))
					return;
			}
			else if (fileExtension == SpriteAnimationAsset::EXTENSION)
			{
				if (!Assert(this, TryCreateAssetFromFile<SpriteAnimationAsset>(file.path()),
					std::format("Tried to add asset at path:{} as sprite animation asset but failed", file.path().string())))
					return;
			}
			else if (fileExtension == SpriteAsset::EXTENSION)
			{
				if (!Assert(this, TryCreateAssetFromFile<SpriteAsset>(file.path()),
					std::format("Tried to add asset at path:{} as sprite asset but failed", file.path().string())))
					return;
			}
			else if (fileExtension == FontAsset::EXTENSION)
			{
				if (!Assert(this, TryCreateAssetFromFile<FontAsset>(file.path()),
					std::format("Tried to add asset at path:{} as font asset but failed", file.path().string())))
					return;
			}
			else if (fileExtension == TextureAsset::EXTENSION)
			{
				if (!Assert(this, TryCreateAssetFromFile<TextureAsset>(file.path()),
					std::format("Tried to add asset at path:{} as texture asset but failed", file.path().string())))
					return;
			}
			else
			{
				LogError(this, std::format("Tried to add asset at path:{} but the extension "
					"does not correspond to any asset action", file.path().string()));
				if (THROW_ON_UNKNWON_ASSET) throw std::invalid_argument("Invalid asset type");
			}
		}
	}

	AssetManager::~AssetManager()
	{
		for (auto& assetPair : m_assets)
		{
			delete assetPair.second;
		}
		m_assets = {};
	}

	bool AssetManager::Validate()
	{
		for (const auto& asset : m_assets)
		{
			if (!Assert(this, asset.second->AreDependenciesSet(), std::format("Tried to validate asset manager but "
				"asset:{} has not all dependencies set!", asset.second->ToString())))
				return false;
		}
		return true;
	}

	std::filesystem::path AssetManager::GetAssetPath(const std::filesystem::path& directoryFile) const
	{
		return ASSET_PATH / directoryFile;
	}

	std::filesystem::path AssetManager::TryGetAssetPath(const std::filesystem::path& fullFileName) const
	{
		return TryGetAssetPath(Asset::ExtractNameFromFile(fullFileName), fullFileName.extension().string());
	}

	std::filesystem::path AssetManager::TryGetAssetPath(const std::string& fileName, const std::string& extension) const
	{
		if (!Assert(this, extension.substr(0, 1) == ".", std::format("Tried to get assed path from file name:{} "
			"and extension but extension is invalid:{}", fileName, extension)))
			return {};

		auto extensionIt = m_allFiles.find(extension);
		if (!Assert(this, extensionIt != m_allFiles.end(), std::format("Tried to get asset path from file:{} extension:{} "
			"but asset manager contains no assets with that extension", fileName, extension)))
			return {};

		for (const auto& path : extensionIt->second)
		{
			if (Asset::ExtractNameFromFile(path) == fileName)
				return path;
		}

		Assert(this, false, std::format("Tried to get asset path from file:{} extension:{} "
			"but asset manager could not find any assets with that file name", fileName, extension));
		return {};
	}

	std::vector<std::string> AssetManager::TryReadAssetFile(const std::string& fileName, const std::string& extension) const
	{
		const std::filesystem::path path = TryGetAssetPath(fileName, extension);
		if (path.empty()) return {};

		return IO::TryReadFileByLine(path);
	}

	bool AssetManager::TryExecuteOnAssetFile(const std::string& fileName, const std::string& extension, const IO::FileLineAction& action) const
	{
		const std::filesystem::path path = TryGetAssetPath(fileName, extension);
		if (!Assert(this, !path.empty(), std::format("Tried to execute action on asset file lines from fileanme:'{}' "
			"and extension:'{}' but resulting asset path is empty", fileName, extension)))
			return false;

		return IO::TryExecuteOnFileByLine(path, action);
	}
	bool AssetManager::TryExecuteOnAssetFile(const std::filesystem::path& fullFileName, const IO::FileLineAction& action) const
	{
		return TryExecuteOnAssetFile(Asset::ExtractNameFromFile(fullFileName), fullFileName.extension().string(), action);
	}

	bool AssetManager::TryWriteToAssetFile(const std::string& fileName, 
		const std::string& extension, const std::string& newContents) const
	{
		const std::filesystem::path path = TryGetAssetPath(fileName, extension);
		if (path.empty()) return false;

		return IO::TryWriteFile(path, newContents);
	}

	Asset* AssetManager::TryGetAssetMutable(const std::string& name)
	{
		auto assetIt = m_assets.find(name);
		if (assetIt == m_assets.end())
			return nullptr;

		return assetIt->second;
	}
	Asset* AssetManager::TryGetAssetMutableFromPath(const std::filesystem::path& path)
	{
		return TryGetAssetMutable(Asset::ExtractNameFromFile(path));
	}

	const Asset* AssetManager::TryGetAsset(const std::string& name) const
	{
		auto assetIt = m_assets.find(name);
		if (assetIt == m_assets.end())
			return nullptr;

		return assetIt->second;
	}
	const Asset* AssetManager::TryGetAssetFromPath(const std::filesystem::path& path) const
	{
		return TryGetAsset(Asset::ExtractNameFromFile(path));
	}
}
