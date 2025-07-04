#include "pch.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Core/Asset/SceneAsset.hpp"
#include "Core/Input/InputProfileAsset.hpp"
#include "Core/Asset/SpriteAnimationAsset.hpp"
#include "Core/Asset/SpriteAsset.hpp"
#include "Core/Asset/FontAsset.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "Utils/Print.hpp"

static bool THROW_ON_UNKNWON_ASSET = false;

namespace AssetManagement
{
	std::filesystem::path AssetManager::ASSET_PATH = "assets";

	AssetManager::AssetManager() : m_assets(), m_runtimeAssets(), m_allFiles() 
	{
		
	}

	AssetManager::~AssetManager()
	{
		for (auto& assetPair : m_assets)
		{
			delete assetPair.second;
		}
		m_assets.clear();

		for (auto& assetPair : m_runtimeAssets)
		{
			delete assetPair.second;
		}
		m_runtimeAssets.clear();
	}

	void AssetManager::Init()
	{
		RegisterAssets<Asset>(nullptr);
	}

	bool AssetManager::Validate()
	{
		for (const auto& asset : m_assets)
		{
			if (!Assert(asset.second->AreDependenciesSet(), std::format("Tried to validate asset manager but "
				"asset:{} has not all dependencies set!", asset.second->ToString())))
				return false;
		}
		return true;
	}

	/// <summary>
	/// PRECONDITION: since this is called from other functions, assetpath is guaranteed to be valid asset path
	/// </summary>
	/// <param name="assetPath"></param>
	/// <param name="outResult"></param>
	/// <returns></returns>
	Asset* AssetManager::TryCreateAssetFromExtension(const std::filesystem::path& assetPath, AssetCreationResult* outResult)
	{
		const std::string fileExtension = assetPath.extension().string();

		Asset* createdAsset = nullptr;
		if (fileExtension == SceneAsset::EXTENSION)
		{
			createdAsset = CreateAssetFromFile<SceneAsset>(assetPath);
		}
		else if (fileExtension == InputProfileAsset::EXTENSION)
		{
			createdAsset = CreateAssetFromFile<InputProfileAsset>(assetPath);
		}
		else if (fileExtension == SpriteAnimationAsset::EXTENSION)
		{
			createdAsset = CreateAssetFromFile<SpriteAnimationAsset>(assetPath);
		}
		else if (fileExtension == SpriteAsset::EXTENSION)
		{
			createdAsset = CreateAssetFromFile<SpriteAsset>(assetPath);
		}
		else if (fileExtension == FontAsset::EXTENSION)
		{
			createdAsset = CreateAssetFromFile<FontAsset>(assetPath);
		}
		else if (fileExtension == TextureAsset::EXTENSION)
		{
			createdAsset = CreateAssetFromFile<TextureAsset>(assetPath);
		}
		else
		{
			//Even if the asset type could not be deduced, we still register to all files
			if (outResult!=nullptr) *outResult = AssetCreationResult::FailedToFindType;
			RegisterAssetToAllFiles(assetPath);
			return nullptr;
		}

		if (outResult != nullptr)
			*outResult = createdAsset != nullptr ? AssetCreationResult::Success : AssetCreationResult::FailedToCreate;
		return createdAsset;
	}

	std::filesystem::path AssetManager::CreateAssetPath(const std::filesystem::path& directoryFile) const
	{
		return ASSET_PATH / directoryFile;
	}
	std::filesystem::path AssetManager::GetRelativeAssetPath(const std::filesystem::path& longerPath) const
	{
		//Note: yes absolute path is redundant, but just in case longer path is already relative to asset, we 
		//can't really figure that out easily, so do it just in cadse
		//LogWarning(std::format("ASSET PATH:{} TARGET PATH:{} NEW PATH:{}", ASSET_PATH.string(), longerPath.string(), std::filesystem::relative(longerPath, ASSET_PATH).string()));
		return std::filesystem::relative(longerPath, ASSET_PATH);
	}
	std::filesystem::path AssetManager::GetAbsoluteAssetPath(const std::filesystem::path& path) const
	{
		return std::filesystem::current_path() / ASSET_PATH / path;
	}

	bool AssetManager::IsValidAssetPath(const std::filesystem::path& relativeAssetPath) const
	{
		return IO::DoesPathExist(GetAbsoluteAssetPath(relativeAssetPath));
	}

	/*std::filesystem::path AssetManager::TryCreateAssetPath(const std::filesystem::path& fullFileName) const
	{
		return TryCreateAssetPath(Asset::ExtractNameFromFile(fullFileName), fullFileName.extension().string());
	}*/

	std::filesystem::path AssetManager::TryCreateAssetPath(const std::string& fileName, const std::string& extension) const
	{
		if (!Assert(extension.substr(0, 1) == ".", std::format("Tried to get assed path from file name:{} "
			"and extension but extension is invalid:{}", fileName, extension)))
			return {};

		auto extensionIt = m_allFiles.find(extension);
		if (!Assert(extensionIt != m_allFiles.end(), std::format("Tried to get asset path from file:{} extension:{} "
			"but asset manager contains no assets with that extension", fileName, extension)))
			return {};

		for (const auto& path : extensionIt->second)
		{
			if (Asset::ExtractNameFromFile(path) == fileName)
				return path;
		}

		Assert(false, std::format("Tried to get asset path from file:{} extension:{} "
			"but asset manager could not find any assets with that file name", fileName, extension));
		return {};
	}

	/*std::vector<std::string> AssetManager::TryReadAssetFile(const std::string& fileName, const std::string& extension) const
	{
		const std::filesystem::path path = TryCreateAssetPath(fileName, extension);
		if (path.empty()) return {};

		return IO::TryReadFileByLine(path);
	}*/

	/*bool AssetManager::TryExecuteOnAssetFile(const std::string& fileName, const std::string& extension, const IO::FileLineAction& action) const
	{
		const std::filesystem::path path = TryCreateAssetPath(fileName, extension);
		if (!Assert(!path.empty(), std::format("Tried to execute action on asset file lines from fileanme:'{}' "
			"and extension:'{}' but resulting asset path is empty", fileName, extension)))
			return false;

		return IO::TryExecuteOnFileByLine(path, action);
	}*/
	bool AssetManager::TryExecuteOnAssetFile(const std::filesystem::path& path, const IO::FileLineAction& action) const
	{
		if (!Assert(IsValidAssetPath(path), std::format("Attempted to execute fil line action on path:{} "
			"but it is not a valid asset path", path.string())))
			return false;

		const std::string extension = path.extension().string();
		auto allFileExtensionIt = m_allFiles.find(extension);
		if (!Assert(allFileExtensionIt != m_allFiles.end(), std::format("Attempted to execute file line action on asset file path:{} "
			"but no asset with this extension exists", path.string())))
			return false;

		auto fileIt = allFileExtensionIt->second.find(path.string());
		if (!Assert(fileIt != allFileExtensionIt->second.end(), std::format("Attempted to execute file line action on asset file path:{} "
			"but no asset file exists in asset manager with that path", path.string())))
			return false;

		return IO::TryExecuteOnFileByLine(GetAbsoluteAssetPath(path), action);
	}

	/*bool AssetManager::TryWriteToAssetFile(const std::string& fileName, 
		const std::string& extension, const std::string& newContents) const
	{
		const std::filesystem::path path = TryCreateAssetPath(fileName, extension);
		if (path.empty()) return false;

		return IO::TryWriteFile(path, newContents);
	}*/

	Asset* AssetManager::TryGetAssetMutable(const std::string& name)
	{
		for (const auto& asset : m_assets)
		{
			if (asset.second->GetName() == name)
				return asset.second;
		}
		return nullptr;
	}
	Asset* AssetManager::TryGetAssetFromPathMutable(const std::filesystem::path& relPath)
	{
		if (!Assert(IsValidAssetPath(relPath), std::format("Attempted to get asset from path:{} MUTABLE"
			"but it is not a valid asset path", relPath.string())))
			return nullptr;

		auto assetIt = m_assets.find(relPath.string());
		if (assetIt == m_assets.end())
		{
			//If the path exists, we attempt to create an asset from the extension since it
			//may exist/ or may have been added recently, after asset manager constructor
			if (IO::DoesPathExist(GetAbsoluteAssetPath(relPath)))
			{
				//LogWarning(std::format("Attempting to create asset from extension when getting asset:{}", relPath.string()));
				return TryCreateAssetFromExtension(relPath);
			}
			else return nullptr;
		}

		return assetIt->second;
	}
	Asset* AssetManager::TryGetRuntimeAssetMutable(const std::string& name)
	{
		auto assetIt = m_runtimeAssets.find(name);
		if (assetIt == m_runtimeAssets.end()) return nullptr;
		return assetIt->second;
	}
}
