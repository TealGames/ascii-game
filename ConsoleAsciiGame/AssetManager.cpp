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
		if (fileExtension == SceneAsset::EXTENSION)
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
		if (Assert(this, asset.second->AreDependenciesSet(), std::format("Tried to validate asset manager but "
			"asset:{} has not all dependencies set!", asset.second->ToString())))
			return false;
	}
	return true;
}

std::filesystem::path AssetManager::GetAssetPath(const std::filesystem::path& directoryFile) const
{
	return ASSET_PATH / directoryFile;
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