#pragma once
#include "Core/Asset/Asset.hpp"
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <functional>
#include <unordered_set>
#include "Utils/HelperFunctions.hpp"
#include "Core/Asset/IDependableAsset.hpp"
#include "Core/IValidateable.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Utils/IOHandler.hpp"

namespace AssetManagement
{
	enum class AssetCreationResult : std::uint8_t
	{
		Success				= 0,
		FailedToCreate		= 1,
		FailedToFindType	= 2
	};

	class AssetManager : public IValidateable
	{
	private:
		/// <summary>
		/// The asset parent folder relative to the location of the executable file
		/// </summary>
		static std::filesystem::path ASSET_PATH;

		//TODO: asset manager should probably get optimized to have faster lookups maybe with 
		//SparseSet, esepcially when doing lookups of the same asset type

		/// <summary>
		// This stores all the assets that have been identified with [asset path, asset]
		// Note: this is heap allocated
		/// </summary>
		std::unordered_map<std::string, Asset*> m_assets;
		/// <summary>
		// This stores all the runtime assets (assets created during runtime -> aka no file path) 
		// that have been identified with [asset name, asset]
		// Note: this is heap allocated
		/// </summary>
		std::unordered_map<std::string, Asset*> m_runtimeAssets;
		/// <summary>
		/// This stores all of the asset extensions, and each of the respective asset paths
		/// [ASSET EXTENSION, ASSET PATH (relative to parent asset path)]
		/// </summary>
		std::unordered_map<std::string, std::unordered_set<std::string>> m_allFiles;
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

			Assert(emplaceSuccess, std::format("Attempted to register asset with argument path:{} (asset path:{})"
				"to all files storage, but it failed to be emplaced", path.string(), path.string()));
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
			//TODO: make sure we create an asset based on the type so we can use polymorphism
			//TODO: check for duplicates 
			/*auto existingIt = m_assets.find(assetPath); 
			if (existingIt != m_assets.end())
				return TryConvertAssetToTypeMutable<T>(existingIt->second);*/

			/*if (!Assert(sameNameAssetIt == m_assets.end(), std::format("Tried to add asset at path:{} "
				"to asset manager but an asset at that path already exists", assetPath.string())))
				return nullptr;*/

			T* existingAsset = TryGetExistingAsset<T>(assetRelPath);
			if (existingAsset != nullptr) return existingAsset;

			//Note: since we need to do io operations on assets, we must use global path (or relative to directory)
			//but global path is easier
			T* assetAsT = new T(GetAbsoluteAssetPath(assetRelPath));
			auto emplaceResult = m_assets.emplace(assetRelPath.string(), assetAsT);
			/*if (!Assert(emplaceResult!= m_assets.end(), std::format("Tried to create asset from file at path:{} "
				"for type:{} but failed to add asset", assetPath.string(), Utils::GetTypeName<T>())))
				return nullptr;*/

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

			try
			{
				return dynamic_cast<T*>(asset);
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to get asset of type:{} name: {} Mutable "
					"but an asset by that name could not be converted to that type. Real Type:{}. Error:{}",
					Utils::GetTypeName<T>(), asset->GetName(), Utils::FormatTypeName(typeid(*(asset)).name()), e.what()));
			}
			return nullptr;
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
					Utils::GetTypeName<T>(), Utils::FormatTypeName(typeid(*(asset)).name()), 
					asset->GetName(), e.what()));
			}
			return nullptr;
		}

	public:
		AssetManager();
		~AssetManager();

		void Init();

		template<typename T>
		requires IsAssetType<T>
		void RegisterAssets(std::vector<T*>* outTypeAssetsCreated)
		{
			if (!Assert(std::filesystem::exists(ASSET_PATH), std::format("Tried to add all assets at path:{} "
				"but path is invalid", ASSET_PATH.string())))
				return;

			std::string assetRelPath = "";
			AssetCreationResult creationResult = AssetCreationResult::FailedToCreate;
			Asset* createdAsset = nullptr;
			for (const auto& file : std::filesystem::recursive_directory_iterator(ASSET_PATH))
			{
				if (!std::filesystem::is_regular_file(file)) continue;

				//TODO: identity asset type from extension now int is placeholder
				assetRelPath = GetRelativeAssetPath(file.path()).string();

				//We add each file based on its extension to the list

				if (!m_assets.empty() && m_assets.find(assetRelPath) != m_assets.end())
					continue;

				//Note: for asset creation, we want the full direct path, but for lookup, we want relative path
				//LogWarning(std::format("registering asset at:{}", assetRelPath));
				createdAsset = TryCreateAssetFromExtension(assetRelPath, & creationResult);

				if (createdAsset == nullptr)
				{
					if (creationResult == AssetCreationResult::FailedToFindType)
						LogWarning(std::format("Tried to add asset at path:'{}' but failed to deduce asset type", file.path().string()));
					else if (creationResult == AssetCreationResult::FailedToCreate)
						LogError(std::format("Tried to add asset at path:'{}' but failed to create asset", file.path().string()));
					else
						Assert(false, std::format("Encountered unidentied asset creation error for asset:'{}'", file.path().string()));
				}
				else
				{
					if (outTypeAssetsCreated != nullptr && typeid(T) == typeid(*createdAsset)) 
						outTypeAssetsCreated->emplace_back(TryConvertAssetToTypeMutable<T>(createdAsset));
					//Log(std::format("Created Asset({}):{}", Utils::FormatTypeName(typeid(*createdAsset).name()), createdAsset->ToString()));
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
		T* TryCreateEmptyAsset(const std::filesystem::path& assetPath)
		{
			if (!Assert(IsValidAssetPath(assetPath), std::format("Attempted to create an empty asset of type:{} at path:{} "
				"but it is not a valid asset path", Utils::GetTypeName<T>(), assetPath.string())))
				return false;

			T* maybeExistingAsset= TryGetExistingAsset<T>(assetPath);
			if (maybeExistingAsset != nullptr)
				return maybeExistingAsset;

			IO::CreatePathIfNotExist(GetAbsoluteAssetPath(assetPath));
			T* assetPtr = CreateAssetFromFile<T>(assetPath);
			//If we have dependencies, we cannot automatically set the data to the file
			if (!static_cast<Asset*>(assetPtr)->AreDependenciesSet()) 
				return assetPtr;

			assetPtr.SaveToSelf();
			return assetPtr;
		}

		/// <summary>
		/// Will copy the asset into asset maanger storage during runtime rather than looking for
		/// the existing file at a certain path to create a wrapper.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="asset"></param>
		template<typename T>
		requires IsAssetType<T> && std::is_copy_constructible_v<T>
		T* CreateRuntimeAsset(const std::string& name, const T& asset)
		{
			T* assetPtr = new T(asset);
			m_runtimeAssets.emplace(name, assetPtr);
			return assetPtr;
		}

		/// <summary>
		/// Combines the directory file within the asset folder
		/// </summary>
		/// <param name="directoryFile"></param>
		/// <returns></returns>
		std::filesystem::path CreateAssetPath(const std::filesystem::path& directoryFile) const;
		std::filesystem::path GetRelativeAssetPath(const std::filesystem::path& longerPath) const;
		std::filesystem::path GetAbsoluteAssetPath(const std::filesystem::path& assetRelativePath) const;
		bool IsValidAssetPath(const std::filesystem::path& relativeAssetPath) const;
		/// <summary>
		/// This is most used for assets that do not have an asset type associated with them
		/// </summary>
		/// <param name="name"></param>
		/// <param name="extension"></param>
		/// <returns></returns>
		//std::filesystem::path TryCreateAssetPath(const std::filesystem::path& fullFileName) const;
		std::filesystem::path TryCreateAssetPath(const std::string& fileName, const std::string& extension) const;

		/// <summary>
		/// Will retrieve the file contents (if found) of the file located at the file and extension names
		/// </summary>
		/// <param name="fileName"></param>
		/// <param name="extension"></param>
		/// <returns></returns>
		//std::vector<std::string> TryReadAssetFile(const std::string& fileName, const std::string& extension) const;
		//bool TryWriteToAssetFile(const std::string& fileName, const std::string& extension, const std::string& newContents) const;
		//bool TryExecuteOnAssetFile(const std::string& fileName, const std::string& extension, const IO::FileLineAction& action) const;
		bool TryExecuteOnAssetFile(const std::filesystem::path& fullPath, const IO::FileLineAction& action) const;

		/// <summary>
		/// Note: this function is slow since it requires iteration through all assets to find one that matches name.
		/// Use the path argument version instead.
		/// </summary>
		/// <param name="name"></param>
		/// <returns></returns>
		Asset* TryGetAssetMutable(const std::string& name);
		Asset* TryGetAssetFromPathMutable(const std::filesystem::path& path);
		Asset* TryGetRuntimeAssetMutable(const std::string& name);

		/// <summary>
		/// Note: this function is slow since it requires iteration through all assets to find one that matches name.
		/// Use the path argument version instead.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="name"></param>
		/// <returns></returns>
		template<typename T>
		requires IsAssetType<T>
		T* TryGetTypeAssetMutable(const std::string& name)
		{
			Asset* maybeAsset = TryGetAssetMutable(name);
			if (maybeAsset == nullptr) return nullptr;

			return TryConvertAssetToTypeMutable<T>(maybeAsset);
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
		T* TryGetRuntimeTypeAssetMutable(const std::string& name)
		{
			Asset* maybeAsset = TryGetRuntimeAssetMutable(name);
			if (maybeAsset == nullptr) return nullptr;

			return TryConvertAssetToTypeMutable<T>(maybeAsset);
		}


		template<typename T>
		requires IsAssetType<T>
		bool IsAssetOfType(const Asset* asset)
		{
			if (asset == nullptr) return false;

			const std::string tTypeName = Utils::GetTypeName<T>();
			return tTypeName == Utils::FormatTypeName(typeid(*asset).name());
		}

		template<typename T>
		requires IsAssetType<T>
		std::vector<T*> GetAssetsOfTypeMutable(const std::function<bool(const Asset&)>& assetPredicate = nullptr)
		{
			std::vector<T*> assets = {};
			const std::string tTypeName = Utils::GetTypeName<T>();
			std::string assetTypeName = "";

			for (auto& asset : m_assets)
			{
				if (asset.second == nullptr) continue;
				assetTypeName = Utils::FormatTypeName(typeid(*(asset.second)).name());

				//LogError(std::format("Checking asset:{} ttype:{} current:{}", asset.second->ToString(), tTypeName, assetTypeName));
				if (tTypeName == assetTypeName && (assetPredicate == nullptr || assetPredicate(*(asset.second))))
				{
					try
					{
						assets.emplace_back(dynamic_cast<T*>(asset.second));
					}
					catch (const std::exception& e)
					{
						LogError(std::format("Tried to get assets of type:{} but asset at path:{} "
							"could not be converted to this type", tTypeName, asset.second->GetPathCopy().string()));
						return {};
					}
				}
			}
			return assets;
		}

		template<typename T>
		requires IsAssetType<T>
		std::vector<T*> GetAssetsOfTypeMutable(const std::filesystem::path& assetDirectory)
		{
			if (!Assert(IsValidAssetPath(assetDirectory), std::format("Attempted to get assets of type:{} from path:{} MUTABLE"
				"but it is not a valid asset path", Utils::GetTypeName<T>(), assetDirectory.string())))
				return {};

			const std::string targetAssetPath = GetAbsoluteAssetPath(assetDirectory).string();
			return GetAssetsOfTypeMutable<T>([&targetAssetPath](const Asset& asset)->bool
				{
					std::string assetpath = asset.GetPath().string();
					//LogError(std::format("Checking path of asset: {} to {}", asset.ToString(), targetAssetPath));
					if (assetpath.size() < targetAssetPath.size()) return false;
					return assetpath.substr(0, targetAssetPath.size()) == targetAssetPath;
				});
		}

		template<typename T, typename ...Args>
		requires IsAssetType<T>&& std::is_base_of_v<IDependableAsset<Args...>, T>
		void InitDependencies(std::add_lvalue_reference_t<Args>... args)
		{
			const std::string tTypeName = Utils::GetTypeName<T>();
			std::vector<T*> assets = GetAssetsOfTypeMutable<T>();
			if (!Assert(!assets.empty(), std::format("Tried to init dependencies for type:{} "
				"but no assets of that type were found", tTypeName)))
				return;

			for (auto& asset : assets)
			{
				static_cast<IDependableAsset<Args...>*>(asset)->SetDependencies(args...);
			}
		}
	};
}


