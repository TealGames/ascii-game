#include "pch.hpp"
#include "AssetManager.hpp"
#include "Debug.hpp"
#include "SceneAsset.hpp"
#include "InputProfileAsset.hpp"

std::filesystem::path AssetManager::ASSET_PATH = "assets";
static bool THROW_ON_UNKNWON_ASSET = false;

AssetManager::AssetManager()
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
			existingExtensionIt->second.push_back(file.path());
		}

		//We try to convert this file into a useable asset
		if (fileExtension == SceneAsset::SCENE_EXTENSION)
		{
			if (!Assert(this, TryAddAsset<SceneAsset>(file.path()), 
				std::format("Tried to add asset at path:{} as scene asset but failed", file.path().string())))
				return;
		}
		else if (fileExtension == InputProfileAsset::EXTENSION)
		{
			if (!Assert(this, TryAddAsset<InputProfileAsset>(file.path()),
				std::format("Tried to add asset at path:{} as input profile asset but failed", file.path().string())))
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

std::filesystem::path AssetManager::TryGetAssetPath(const std::filesystem::path& fullFileName)
{
	return TryGetAssetPath(Asset::ExtractNameFromFile(fullFileName), fullFileName.extension().string());
}

std::filesystem::path AssetManager::TryGetAssetPath(const std::string& fileName, const std::string& extension)
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

Asset* AssetManager::TryGetAssetMutable(const std::string& name)
{
	auto assetIt= m_assets.find(name);
	if (assetIt == m_assets.end())
		return nullptr;

	return assetIt->second;
}
const Asset* AssetManager::TryGetAsset(const std::string& name) const
{
	auto assetIt = m_assets.find(name);
	if (assetIt == m_assets.end()) 
		return nullptr;

	return assetIt->second;
}