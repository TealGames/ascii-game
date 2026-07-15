#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>
#include <functional>
#include "Utils/HelperFunctions.hpp"
#include "Core/Asset/Asset.hpp"
#include "Utils/Debug.hpp"
#include "Utils/IOHandler.hpp"
#include "Utils/ToStringFunctions.hpp"
#include "Core/IValidateable.hpp"

namespace Engine::Core { class EngineState; }
namespace Engine::Assets
{
	enum class AssetCreationResult : std::uint8_t
	{
		Success				= 0,
		FailedToCreate		= 1,
		FailedToFindType	= 2
	};

	using AssetCreationFunction = std::function<Asset*(const std::filesystem::path&)>;
	class AssetManager : public IValidateable
	{
	private:
		static constexpr bool PREVENT_HIDDEN_ASSET_LOOKUP = true;

		//TODO: asset manager should probably get optimized to have faster lookups maybe with 
		//SparseSet, esepcially when doing lookups of the same asset type

		/// <summary>
		// This stores all the assets that have been identified with [asset path, asset]
		// Note: this is heap allocated
		/// </summary>
		std::unordered_map<std::string, Asset*> m_assets;
		std::unordered_map<ObjectID, Asset*> m_assetIds;
		/// <summary>
		/// This stores all of the asset extensions, and each of the respective asset paths
		/// [ASSET EXTENSION, ASSET PATH (relative to parent asset path)]
		/// </summary>
		std::unordered_map<std::string, std::unordered_set<std::string>> m_allFiles;
		std::unordered_map<std::string_view, AssetCreationFunction> m_extensionAssetCreators;

		static std::filesystem::path m_assetPath;
		/// <summary>
		/// The assets that are hidden and cannot be retrieved from asset lookup functions
		/// and if set while registering assets, will be skipped when creating asset wrappers
		/// </summary>
		static std::unordered_set<std::string> m_hiddenAssetPaths;

		Core::EngineState* m_engineState;
	public:

	private:
		/// <summary>
		/// PRECONDITION: since this is called from other functions, path is a valid asset path
		/// </summary>
		/// <param name="path"></param>
		void RegisterAssetToAllFiles(const std::filesystem::path& path)
		{
			std::string fileExtension = path.extension().string();
			auto allFileIt = m_allFiles.find(fileExtension);
			bool emplaceSuccess = false;
			//Note: while global paths would be better, we want to save space since we know
			//all assets must be in one parent folder
			if (allFileIt == m_allFiles.end())
			{
				emplaceSuccess = m_allFiles.emplace(fileExtension, std::unordered_set<std::string>{path.string()}).second;
			}
			else
			{
				emplaceSuccess = allFileIt->second.emplace(path.string()).second;
			}

			ENGINE_ASSERT(emplaceSuccess, "Attempted to register asset with argument path:{} (asset path:{})"
				"to all files storage, but it failed to be emplaced", path.string(), path.string());
		}

		/// <summary>
		/// Will create an asset wrapper based on the file found at the path
		/// that will store the data
		/// PRECONDITION: since this is called from other functions, path is a valid asset path
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="assetPath"></param>
		/// <returns></returns>
		template<typename T>
		requires IsAssetType<T>
		T* CreateAssetFromFile(const std::filesystem::path& assetRelPath)
		{
			T* existingAsset = TryGetExistingAsset<T>(assetRelPath);
			if (existingAsset != nullptr) return existingAsset;

			//Note: since we need to do io operations on assets, we must use global path (or relative to directory)
			//but global path is easier
			T* assetAsT = new T(GetAbsoluteAssetPath(assetRelPath));
			Asset* assetBase = static_cast<Asset*>(assetAsT);

			auto emplaceResult = m_assets.emplace(assetRelPath.string(), assetAsT);
			ENGINE_ASSERT(m_engineState != nullptr, "Tried to create asset from file at path: {} in asset manager but engine state is null. "
				"This means an asset was attempted to be created before asset manager was init", assetRelPath.string());

			assetBase->SetDependencies(*m_engineState);
			RegisterAssetToAllFiles(assetRelPath);
			return assetAsT;
		}

		Asset* TryCreateAssetFromExtension(const std::filesystem::path& assetRelPath, AssetCreationResult* outResult=nullptr);

		/// <summary>
		/// PRECONDITION: since this is called from other functions, path is a valid asset path
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="assetPath"></param>
		/// <returns></returns>
		template<typename T>
		requires IsAssetType<T>
		T* TryGetExistingAsset(const std::filesystem::path& assetRelPath)
		{
			auto it = m_assets.find(assetRelPath.string());
			if (it != m_assets.end()) return TryConvertAssetToTypeMutable<T>(it->second);
			return nullptr;
		}

		template<typename T>
		requires IsAssetType<T>
		T* TryConvertAssetToTypeMutable(Asset* asset) const
		{
			if (asset == nullptr) 
				return nullptr;

			if (::Utils::ToStringTypeName<T>() != ::Utils::FormatTypeName(typeid(*(asset)).name()))
			{
				LogError(std::format("Tried to get asset of type:'{}' name:'{}' Mutable "
					"but an asset by that name could not be converted to that type. Real Type:'{}'",
					::Utils::ToStringTypeName<T>(), asset->GetName(), ::Utils::FormatTypeName(typeid(*(asset)).name())));
				return nullptr;
			}

			return dynamic_cast<T*>(asset);
		}

		template<typename T>
		requires IsAssetType<T>
		const T* TryConvertAssetToType(const Asset* asset) const
		{
			if (asset == nullptr)
				return nullptr;

			try
			{
				return dynamic_cast<const T*>(asset);
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to get asset of type:{} by name:{} IMMUTABLE "
					"but an asset by that name could not be converted to the type:{}. Error:{}",
					::Utils::ToStringTypeName<T>(), ::Utils::FormatTypeName(typeid(*(asset)).name()),
					asset->GetName(), e.what()));
			}
			return nullptr;
		}

	public:
		AssetManager();
		~AssetManager();

		void Init(Core::EngineState& state, const std::filesystem::path& assetPath);
		
		/// <summary>
		/// Note: the path has to be an asset parent directory path
		/// </summary>
		/// <param name="path"></param>
		/// <param name="doHide"></param>
		static void SetAssetHiddenStatus(const std::filesystem::path& path, const bool doHide);
		/// <summary>
		/// Note: path has to be valid asset parent directory path
		/// </summary>
		/// <param name="path"></param>
		/// <returns></returns>
		bool IsAssetHiddenFromPath(std::filesystem::path path, const bool isAbsolutePath) const;
		/// <summary>
		/// Note: this version requires iteration through all assets. Use path version instead.
		/// </summary>
		/// <param name="name"></param>
		/// <returns></returns>
		bool IsAssetHidden(const std::string& name) const;

		template<typename T>
		requires IsAssetType<T>
		void RegisterAssets(std::vector<T*>* outTypeAssetsCreated)
		{
			if (!Assert(::Utils::IO::DoesPathExist(m_assetPath), "Tried to add all assets at path:{} "
				"but path is invalid", m_assetPath.string()))
				return;

			std::string assetRelPath = "";
			AssetCreationResult creationResult = AssetCreationResult::FailedToCreate;
			Asset* createdAsset = nullptr;
			for (const auto& file : std::filesystem::recursive_directory_iterator(m_assetPath))
			{
				if (!std::filesystem::is_regular_file(file)) continue;

				assetRelPath = GetRelativeAssetPath(file.path()).string();

				/*LogWarning(std::format("Is asset path:{} hidden:{}({}) VALUE:{}", assetRelPath, 
					Utils::ToStringIterable<std::unordered_set<std::string>, std::string>(m_hiddenAssetPaths), m_hiddenAssetPaths.size(), 
					IsAssetHiddenFromPath(assetRelPath, false)));*/
				if (IsAssetHiddenFromPath(assetRelPath, false))
				{
					LogWarning(std::format("Skipping hidden asset at path:{}", assetRelPath));
					continue;
				}
				
				//If this asset already is registered (if an asset ), skip
				if (!m_assets.empty() && m_assets.find(assetRelPath) != m_assets.end())
					continue;

				//Note: for asset creation, we want the full direct path, but for lookup, we want relative path
				//LogWarning(std::format("registering asset at:{}", assetRelPath));
				createdAsset = TryCreateAssetFromExtension(assetRelPath, &creationResult);

				if (createdAsset == nullptr)
				{
					if (creationResult == AssetCreationResult::FailedToFindType)
						LogWarning(std::format("Tried to add asset at path:'{}' but failed to deduce asset type", file.path().string()));
					else if (creationResult == AssetCreationResult::FailedToCreate)
						LogError(std::format("Tried to add asset at path:'{}' but failed to create asset", file.path().string()));
					else
						Assert(false, "Encountered unidentied asset creation error for asset:'{}'", file.path().string());
				}
				else
				{
					if (outTypeAssetsCreated != nullptr && typeid(T) == typeid(*createdAsset)) 
						outTypeAssetsCreated->emplace_back(TryConvertAssetToTypeMutable<T>(createdAsset));
					//Log(std::format("Created Asset({}):{}", ::Utils::FormatTypeName(typeid(*createdAsset).name()), createdAsset->ToString()));
				}
			}
		}

		bool Validate() override;

		/// <summary>
		/// Will create a blank new asset of the type at the specific path
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		template<typename T>
		requires IsAssetType<T>
		T* TryCreateOrGetAsset(const std::filesystem::path& assetPath, bool* outHadAssetFile = nullptr)
		{
			//NOTE: we use the get existing variant of asset retrieval so if asset is NOT
			//found then it is not created automatically 
			T* maybeExistingAsset= TryGetExistingAsset<T>(assetPath);
			if (maybeExistingAsset != nullptr)
			{
				if (outHadAssetFile != nullptr) *outHadAssetFile = true;
				return maybeExistingAsset;
			}

			bool createdAssetFile = ::Utils::IO::CreatePathIfNotFound(GetAbsoluteAssetPath(assetPath));
			if (outHadAssetFile != nullptr) *outHadAssetFile = !createdAssetFile;

			T* assetPtr = CreateAssetFromFile<T>(assetPath);
			assetPtr->SaveToSelf();
			return assetPtr;
		}

		/// <summary>
		/// Combines the directory file within the asset folder
		/// </summary>
		/// <param name="directoryFile"></param>
		/// <returns></returns>
		static std::filesystem::path CreateAssetPath(const std::filesystem::path& directoryFile);
		static std::filesystem::path GetRelativeAssetPath(const std::filesystem::path& longerPath);
		static std::filesystem::path GetAbsoluteAssetPath(const std::filesystem::path& assetRelativePath);
		static bool IsValidAssetPath(const std::filesystem::path& relativeAssetPath);
		std::filesystem::path TryCreateAssetPath(const std::string& fileName, const std::string& extension) const;

		bool TryExecuteOnAssetFile(const std::filesystem::path& fullPath, const ::Utils::IO::FileLineAction& action) const;

		Asset* TryGetAssetFromPathMutable(const std::filesystem::path& path);
		Asset* TryGetAssetFromIdMutable(ObjectID id);

		/// <summary>
		/// Note: this function is slow since it requires iteration through all assets to find one that matches name.
		/// Use the path argument version instead.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="name"></param>
		/// <returns></returns>
		template<typename T>
		requires IsAssetType<T>
		T* TryGetTypeAssetFromLiteralMutable(const char* name)
		{
			for (const auto& asset : m_assets)
			{
				/*LogWarning(std::format("checking asset:'{}' for target:'{}' comp:{}", asset.second->GetName(), name,
					strncmp(asset.second->GetName().c_str(), name, asset.second->GetName().size())));*/

				if (strncmp(asset.second->GetName().c_str(), name, asset.second->GetName().size()) != 0 || 
					::Utils::ToStringTypeName<T>() != ::Utils::FormatTypeName(typeid(*asset.second).name()))
					continue;

				if (PREVENT_HIDDEN_ASSET_LOOKUP)
				{
					const std::filesystem::path relPath = GetRelativeAssetPath(asset.second->GetAbsolutePath());
					if (IsAssetHiddenFromPath(relPath, false))
					{
						LogWarning(std::format("Attempted to get asset by path:{} but this asset was marked as hidden", relPath.string()));
						return nullptr;
					}
				}

				//LogWarning(std::format("Returning shader:{}", asset.second->ToString()));
				return TryConvertAssetToTypeMutable<T>(asset.second);
			}
			return nullptr;
		}

		template<typename T>
		requires IsAssetType<T>
		T* TryGetTypeAssetFromPathMutable(const std::filesystem::path& path)
		{
			Asset* maybeAsset = TryGetAssetFromPathMutable(path);
			if (maybeAsset == nullptr) return nullptr;

			return TryConvertAssetToTypeMutable<T>(maybeAsset);
		}
		template<typename T>
		requires IsAssetType<T>
		T* TryGetTypeAssetFromIdMutable(ObjectID id)
		{
			Asset* maybeAsset = TryGetAssetFromIdMutable(id);
			if (maybeAsset == nullptr) return nullptr;

			return TryConvertAssetToTypeMutable<T>(maybeAsset);
		}


		template<typename T>
		requires IsAssetType<T>
		bool IsAssetOfType(const Asset* asset)
		{
			if (asset == nullptr) return false;

			const std::string tTypeName = ::Utils::ToStringTypeName<T>();
			return tTypeName == ::Utils::FormatTypeName(typeid(*asset).name());
		}

		template<typename T>
		requires IsAssetType<T>
		std::vector<T*> GetAssetsOfTypeMutable(const std::function<bool(const Asset&)>& assetPredicate = nullptr)
		{
			std::vector<T*> assets = {};
			for (auto& asset : m_assets)
			{
				if (asset.second == nullptr) continue;
				if (T* tPtr = dynamic_cast<T*>(asset.second))
				{
					if (assetPredicate == nullptr || assetPredicate(*asset.second))
						assets.emplace_back(tPtr);
				}
			}
			return assets;
		}

		template<typename T>
		requires IsAssetType<T>
		std::vector<T*> GetAssetsOfTypeMutable(const std::filesystem::path& assetDirectory)
		{
			if (!Assert(IsValidAssetPath(assetDirectory), "Attempted to get assets of type:{} from path:{} MUTABLE"
				"but it is not a valid asset path", ::Utils::ToStringTypeName<T>(), assetDirectory.string()))
				return {};

			const std::string targetAssetPath = GetAbsoluteAssetPath(assetDirectory).string();
			return GetAssetsOfTypeMutable<T>([&targetAssetPath](const Asset& asset)->bool
				{
					std::string assetpath = asset.GetAbsolutePath().string();
					if (targetAssetPath.find("scenes") != -1)
						LogWarning(std::format("Found scene: {}", assetpath));

					
					//LogError(std::format("Checking path of asset: {} to {}", asset.ToString(), targetAssetPath));
					if (assetpath.size() < targetAssetPath.size()) return false;
					const bool matchesPath = assetpath.substr(0, targetAssetPath.size()) == targetAssetPath;
					if (targetAssetPath.find("scenes") != -1) LogWarning(std::format("Matches path: {}", matchesPath));

					return matchesPath;
				});
		}

		/*
		template<typename T, typename ...Args>
		requires IsAssetType<T> && std::is_base_of_v<IDependableAsset<Args...>, T>
		void InitDependencies(std::add_lvalue_reference_t<Args>... args)
		{
			const std::string tTypeName = ::Utils::ToStringTypeName<T>();
			std::vector<T*> assets = GetAssetsOfTypeMutable<T>();
			if (!Assert(!assets.empty(), "Tried to init dependencies for type:{} "
				"but no assets of that type were found", tTypeName))
				return;

			for (auto& asset : assets)
			{
				static_cast<IDependableAsset<Args...>*>(asset)->SetDependencies(args...);
			}
		}
		*/
	};
}


