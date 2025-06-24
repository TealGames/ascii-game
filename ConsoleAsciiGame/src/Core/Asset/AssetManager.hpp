#pragma once
#include "Asset.hpp"
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <functional>
#include "HelperFunctions.hpp"
#include "IDependableAsset.hpp"
#include "IValidateable.hpp"
#include "Debug.hpp"
#include "IOHandler.hpp"

namespace AssetManagement
{
	class AssetManager : public IValidateable
	{
	private:
		static std::filesystem::path ASSET_PATH;

		//TODO: asset manager should probably get optimized to have faster lookups maybe with 
		//SparseSet, esepcially when doing lookups of the same asset type

		/// <summary>
		// This stores all the assets that have been identified with [asset name, asset]
		// Note: this is heap allocated
		/// </summary>
		std::unordered_map<std::string, Asset*> m_assets;
		/// <summary>
		/// This stores all of the files in the asset folder with all the paths with the extension
		/// in [ASSET SUBFOLDER NAME, ASSET PATHS]
		/// </summary>
		std::unordered_map<std::string, std::vector<std::filesystem::path>> m_allFiles;
	public:

	private:
		/// <summary>
		/// Will create an asset wrapper based on the file found at the path
		/// that will store the data
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="assetPath"></param>
		/// <returns></returns>
		template<typename T>
		requires IsAssetType<T>
		T* TryCreateAssetFromFile(const std::filesystem::path& assetPath)
		{
			//TODO: make sure we create an asset based on the type so we can use polymorphism
			//TODO: check for duplicates 
			std::string newAssetName = Asset::ExtractNameFromFile(assetPath);
			auto sameNameAssetIt = m_assets.find(newAssetName);
			if (!Assert(this, sameNameAssetIt == m_assets.end(), std::format("Tried to add asset at path:{} "
				"to asset manager but an asset with name:{} already exists", assetPath.string(), newAssetName)))
				return nullptr;

			T* assetAsT = new T(assetPath);
			const auto emplaceResult = m_assets.emplace(assetAsT->GetName(), assetAsT);
			if (!Assert(this, emplaceResult.second, std::format("Tried to create asset from file at path:{} "
				"for type:{} but failed to add asset", assetPath.string(), Utils::GetTypeName<T>())))
				return nullptr;

			return assetAsT;
		}

	public:
		AssetManager();
		~AssetManager();

		bool Validate() override;

		/// <summary>
		/// Will create a blank new asset of the type at the specific path
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		template<typename T>
		requires IsAssetType<T>
		T* TryCreateNewAsset(const std::filesystem::path& assetPath)
		{
			std::string newAssetName = Asset::ExtractNameFromFile(assetPath);
			auto sameNameAssetIt = m_assets.find(newAssetName);
			if (!Assert(this, sameNameAssetIt == m_assets.end(), std::format("Tried to create asset at path:{} "
				"to asset manager but an asset with name:{} already exists", assetPath.string(), newAssetName)))
				return nullptr;

			IO::CreatePathIfNotExist(assetPath);
			T* assetPtr = TryCreateAssetFromFile<T>(assetPath);
			//If we have dependencies, we cannot automatically set the data to the file
			if (!static_cast<Asset*>(assetPtr)->AreDependenciesSet()) 
				return assetPtr;

			assetPtr.SaveToSelf();
			return assetPtr;
		}

		/// <summary>
		/// Combines the directory file within the asset folder
		/// </summary>
		/// <param name="directoryFile"></param>
		/// <returns></returns>
		std::filesystem::path GetAssetPath(const std::filesystem::path& directoryFile) const;

		/// <summary>
		/// This is most used for assets that do not have an asset type associated with them
		/// </summary>
		/// <param name="name"></param>
		/// <param name="extension"></param>
		/// <returns></returns>
		std::filesystem::path TryGetAssetPath(const std::filesystem::path& fullFileName) const;
		std::filesystem::path TryGetAssetPath(const std::string& fileName, const std::string& extension) const;

		/// <summary>
		/// Will retrieve the file contents (if found) of the file located at the file and extension names
		/// </summary>
		/// <param name="fileName"></param>
		/// <param name="extension"></param>
		/// <returns></returns>
		std::vector<std::string> TryReadAssetFile(const std::string& fileName, const std::string& extension) const;
		bool TryWriteToAssetFile(const std::string& fileName, const std::string& extension, const std::string& newContents) const;
		bool TryExecuteOnAssetFile(const std::string& fileName, const std::string& extension, const IO::FileLineAction& action) const;
		bool TryExecuteOnAssetFile(const std::filesystem::path& fullFileName, const IO::FileLineAction& action) const;

		const Asset* TryGetAsset(const std::string& name) const;
		const Asset* TryGetAssetFromPath(const std::filesystem::path& path) const;

		template<typename T>
		requires IsAssetType<T>
		const T* TryGetTypeAsset(const std::string& name) const
		{
			const Asset* maybeAsset = TryGetAsset(name);
			if (maybeAsset == nullptr) return nullptr;

			const std::string tTypeName = Utils::GetTypeName<T>();
			const std::string realTypename = Utils::FormatTypeName(typeid(*(maybeAsset)).name());
			if (!Assert(this, tTypeName == realTypename, std::format("Tried to get asset of type:{} "
				"by name:{} IMMUTABLE but asset type is actually:{}", tTypeName, name, realTypename)))
				return nullptr;

			try
			{
				return dynamic_cast<const T*>(maybeAsset);
			}
			catch (const std::exception& e)
			{
				LogError(this, std::format("Tried to get asset of type:{} by name:{} IMMUTABLE "
					"but an asset by that name could not be converted to the type. Error:{}",
					tTypeName, maybeAsset->GetName(), e.what()));
				return nullptr;
			}
			return nullptr;
		}
		template<typename T>
		requires IsAssetType<T>
		const T* TryGetTypeAssetFromPath(const std::filesystem::path& path) const
		{
			return TryGetAsset<T>(Asset::ExtractNameFromFile(path));
		}

		Asset* TryGetAssetMutable(const std::string& name);
		Asset* TryGetAssetMutableFromPath(const std::filesystem::path& path);

		template<typename T>
		requires IsAssetType<T>
		T* TryGetTypeAssetMutable(const std::string& name)
		{
			Asset* maybeAsset = TryGetAssetMutable(name);
			if (maybeAsset == nullptr) return nullptr;

			const std::string tTypeName = Utils::GetTypeName<T>();
			const std::string realTypename = Utils::FormatTypeName(typeid(*(maybeAsset)).name());
			if (!Assert(this, tTypeName == realTypename, std::format("Tried to get asset of type:{} "
				"by name:{} MUTABLE but asset type is actually:{}", tTypeName, name, realTypename)))
				return nullptr;

			try
			{
				return dynamic_cast<T*>(maybeAsset);
			}
			catch (const std::exception& e)
			{
				LogError(this, std::format("Tried to get asset of type:{} by name:{} MUTABLE "
					"but an asset by that name could not be converted to the type. Error:{}", 
					tTypeName, maybeAsset->GetName(), e.what()));
				return nullptr;
			}
			return nullptr;
		}
		template<typename T>
		requires IsAssetType<T>
		T* TryGetTypeAssetFromPathMutable(const std::filesystem::path& path)
		{
			return TryGetAssetMutable<T>(Asset::ExtractNameFromFile(path));
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
		std::vector<T*> GetAssetsOfTypeMutable(const std::function<bool(const Asset&)> assetPredicate = nullptr)
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
						LogError(this, std::format("Tried to get assets of type:{} but asset at path:{} "
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
			const std::string targetAssetPath = GetAssetPath(assetDirectory).string();
			return GetAssetsOfTypeMutable<T>([&targetAssetPath](const Asset& asset)->bool
				{
					std::string assetpath = asset.GetPathCopy().string();
					//LogError(std::format("Checking path of asset: {} to {}", asset.ToString(), targetAssetPath));
					if (assetpath.size() < targetAssetPath.size()) return false;
					return assetpath.substr(0, targetAssetPath.size()) == targetAssetPath;
				});
		}

		template<typename T, typename ...Args>
		requires IsAssetType<T>&& std::is_base_of_v<IDependable<Args...>, T>
		void InitDependencies(std::add_lvalue_reference_t<Args>... args)
		{
			const std::string tTypeName = Utils::GetTypeName<T>();
			std::vector<T*> assets = GetAssetsOfTypeMutable<T>();
			if (!Assert(this, !assets.empty(), std::format("Tried to init dependencies for type:{} "
				"but no assets of that type were found", tTypeName)))
				return;

			for (auto& asset : assets)
			{
				static_cast<IDependable<Args...>*>(asset)->SetDependencies(args...);
			}
		}
	};
}


