#include "pch.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Utils/Debug.hpp"
#include "Core/Asset/AssetMacros.hpp"
#include "Core/Asset/AllAssetsHeader.hpp"

static constexpr bool THROW_ON_UNKNWON_ASSET = false;

namespace Engine::Assets
{
#define ADD_ASSET_CREATOR_ENTRIES(Namespace, AssetName) \
for (const auto& extension : Namespace::AssetName::EXTENSIONS) \
{\
	m_extensionAssetCreators.insert({extension, [this](const std::filesystem::path& assetPath) \
		{ return CreateAssetFromFile<Namespace::AssetName>(assetPath);} }); \
}\

	std::unordered_set<std::string> AssetManager::m_hiddenAssetPaths = {};
	std::filesystem::path AssetManager::m_assetPath = {};

	AssetManager::AssetManager() : m_assets(), m_allFiles(), m_assetIds(), m_extensionAssetCreators(), m_engineState(nullptr)
	{
		ALL_ASSET_MACRO(ADD_ASSET_CREATOR_ENTRIES)
	}

	AssetManager::~AssetManager()
	{
		for (auto& assetPair : m_assets)
		{
			delete assetPair.second;
		}
		m_assets.clear();
		m_assetIds.clear();
	}

	void AssetManager::Init(Core::EngineState& state, const std::filesystem::path& assetPath)
	{
		m_assetPath = assetPath;
		m_engineState = &state;
		RegisterAssets<Asset>(nullptr);
	}

	bool AssetManager::Validate()
	{
		return true;
	}

	void AssetManager::SetAssetHiddenStatus(const std::filesystem::path& path, const bool doHide)
	{
		if (!Assert(IsValidAssetPath(path), "Attempted to set asset hidden status with path:{} "
			"but it is not a valid asset path", path.string()))
			return;

		const std::filesystem::path relPath = GetRelativeAssetPath(path);
		if (doHide) m_hiddenAssetPaths.insert(relPath.string());
		else m_hiddenAssetPaths.erase(relPath.string());
	}
	bool AssetManager::IsAssetHiddenFromPath(std::filesystem::path path, const bool isAbsolutePath) const
	{
		if (m_hiddenAssetPaths.empty()) return false;
		if (!Assert(IsValidAssetPath(path), "Attempted to get whether asset is hidden at path:{} "
			"but it is not a valid asset path", path.string()))
			return false;

		if (isAbsolutePath) path = GetRelativeAssetPath(path);
		return m_hiddenAssetPaths.find(path.string()) != m_hiddenAssetPaths.end();
	}
	bool AssetManager::IsAssetHidden(const std::string& name) const
	{
		for (const auto& relPath : m_hiddenAssetPaths)
		{
			if (std::filesystem::path(relPath).stem() == name)
				return true;
		}
		return false;
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
		auto assetCreatorFuncIt = m_extensionAssetCreators.find(std::string_view(fileExtension));
		if (assetCreatorFuncIt == m_extensionAssetCreators.end())
		{
			//Even if the asset type could not be deduced, we still register to all files
			if (outResult != nullptr) *outResult = AssetCreationResult::FailedToFindType;
			RegisterAssetToAllFiles(assetPath);
			return nullptr;
		}
		createdAsset = assetCreatorFuncIt->second(assetPath);

		if (outResult != nullptr)
			*outResult = createdAsset != nullptr ? AssetCreationResult::Success : AssetCreationResult::FailedToCreate;
		return createdAsset;
	}

	std::filesystem::path AssetManager::CreateAssetPath(const std::filesystem::path& directoryFile)
	{
		return m_assetPath / directoryFile;
	}
	std::filesystem::path AssetManager::GetRelativeAssetPath(const std::filesystem::path& longerPath)
	{
		//Note: yes absolute path is redundant, but just in case longer path is already relative to asset, we 
		//can't really figure that out easily, so do it just in cadse
		//LogWarning(std::format("ASSET PATH:{} TARGET PATH:{} NEW PATH:{}", ASSET_PATH.string(), longerPath.string(), std::filesystem::relative(longerPath, ASSET_PATH).string()));
		return std::filesystem::relative(longerPath, m_assetPath);
	}
	std::filesystem::path AssetManager::GetAbsoluteAssetPath(const std::filesystem::path& path)
	{
		return std::filesystem::current_path() / m_assetPath / path;
	}

	bool AssetManager::IsValidAssetPath(const std::filesystem::path& relativeAssetPath)
	{
		return ::Utils::IO::DoesPathExist(GetAbsoluteAssetPath(relativeAssetPath));
	}

	std::filesystem::path AssetManager::TryCreateAssetPath(const std::string& fileName, const std::string& extension) const
	{
		if (!Assert(extension.substr(0, 1) == ".", "Tried to get assed path from file name:{} "
			"and extension but extension is invalid:{}", fileName, extension))
			return {};

		auto extensionIt = m_allFiles.find(extension);
		if (!Assert(extensionIt != m_allFiles.end(), "Tried to get asset path from file:{} extension:{} "
			"but asset manager contains no assets with that extension", fileName, extension))
			return {};

		for (const auto& path : extensionIt->second)
		{
			if (Asset::ExtractNameFromFile(path) == fileName)
				return path;
		}

		Assert(false, "Tried to get asset path from file:{} extension:{} "
			"but asset manager could not find any assets with that file name", fileName, extension);
		return {};
	}

	bool AssetManager::TryExecuteOnAssetFile(const std::filesystem::path& path, const ::Utils::IO::FileLineAction& action) const
	{
		if (!Assert(IsValidAssetPath(path), "Attempted to execute fil line action on path:{} "
			"but it is not a valid asset path", path.string()))
			return false;

		const std::string extension = path.extension().string();
		auto allFileExtensionIt = m_allFiles.find(extension);
		if (!Assert(allFileExtensionIt != m_allFiles.end(), "Attempted to execute file line action on asset file path:{} "
			"but no asset with this extension exists", path.string()))
			return false;

		auto fileIt = allFileExtensionIt->second.find(path.string());
		if (!Assert(fileIt != allFileExtensionIt->second.end(), "Attempted to execute file line action on asset file path:{} "
			"but no asset file exists in asset manager with that path", path.string()))
			return false;

		return ::Utils::IO::TryExecuteOnFileByLine(GetAbsoluteAssetPath(path), action);
	}

	Asset* AssetManager::TryGetAssetFromPathMutable(const std::filesystem::path& relPath)
	{
		if (!Assert(IsValidAssetPath(relPath), "Attempted to get asset from path:{} MUTABLE"
			"but it is not a valid asset path", relPath.string()))
			return nullptr;

		if (PREVENT_HIDDEN_ASSET_LOOKUP && IsAssetHiddenFromPath(relPath, false))
		{
			LogWarning(std::format("Attempted to get asset by path:{} but this asset was marked as hidden", relPath.string()));
			return nullptr;
		}

		auto assetIt = m_assets.find(relPath.string());
		if (assetIt == m_assets.end())
		{
			//If the path exists, we attempt to create an asset from the extension since
			//this asset may not have been registered yet (either because it is before the register assets function call
			// OR maybe this was invoked by one asset type during the register process, either way we still want to allow
			//asset retrieval)
			if (::Utils::IO::DoesPathExist(GetAbsoluteAssetPath(relPath)))
			{
				LogWarning(std::format("Attempting to create asset from extension when getting asset:{}", relPath.string()));
				return TryCreateAssetFromExtension(relPath);
			}
			else return nullptr;
		}

		return assetIt->second;
	}

	Asset* AssetManager::TryGetAssetFromIdMutable(ObjectID id)
	{
		if (id == INVALID_ID)
		{
			LogWarning(std::format("Attempted to get asset by invalid id"));
			return nullptr;
		}

		auto assetIt = m_assetIds.find(id);
		if (assetIt == m_assetIds.end()) 
			return nullptr;

		return assetIt->second;
	}
}
