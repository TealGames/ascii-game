#pragma once
#include "Asset.hpp"
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <functional>
#include "HelperFunctions.hpp"
#include "IDependable.hpp"
#include "IValidateable.hpp"
#include "Debug.hpp"

template<typename T>
concept IsAssetType = std::is_base_of_v<Asset, T>;

class AssetManager : public IValidateable
{
private:
	static std::filesystem::path ASSET_PATH;

	/// <summary>
	// This stores all the assets that have been identified with [asset name, asset]
	// Note: this is heap allocated
	/// </summary>
	std::unordered_map<std::string, Asset*> m_assets;
	/// <summary>
	/// This stores all of the files in the asset folder with all the paths with the extension
	/// </summary>
	std::unordered_map<std::string, std::vector<std::filesystem::path>> m_allFiles;
public:

private:
public:
	AssetManager();
	~AssetManager();

	bool Validate() override;

	/// <summary>
	/// Combines the directory file within the asset folder
	/// </summary>
	/// <param name="directoryFile"></param>
	/// <returns></returns>
	std::filesystem::path GetAssetPath(const std::filesystem::path& directoryFile) const;

	template<typename T>
	requires IsAssetType<T>
	bool TryAddAsset(const std::filesystem::path& assetPath)
	{
		//TODO: make sure we create an asset based on the type so we can use polymorphism
		//TODO: check for duplicates 
		std::string newAssetName = Asset::ExtractNameFromFile(assetPath);
		auto sameNameAssetIt = m_assets.find(newAssetName);
		if (!Assert(this, sameNameAssetIt == m_assets.end(), std::format("Tried to add asset at path:{} "
			"to asset manager but an asset with namne:{} already exists", assetPath.string(), newAssetName)))
			return false;

		Asset* newAsset = new T(assetPath);
		m_assets.emplace(newAsset->GetName(), newAsset);
		return true;
	}

	/// <summary>
	/// This is most used for assets that do not have an asset type associated with them
	/// </summary>
	/// <param name="name"></param>
	/// <param name="extension"></param>
	/// <returns></returns>
	std::filesystem::path TryGetAssetPath(const std::filesystem::path& fullFileName);
	std::filesystem::path TryGetAssetPath(const std::string& fileName, const std::string& extension);

	Asset* TryGetAssetMutable(const std::string& name);
	const Asset* TryGetAsset(const std::string& name) const;

	template<typename T>
	T* TryGetAssetMutable(const std::string& name)
	{
		Asset* maybeAsset = TryGetAssetMutable(name);
		if (maybeAsset == nullptr) return nullptr;

		const std::string tTypeName = Utils::GetTypeName<T>();
		const std::string realTypename = Utils::FormatTypeName(typeid(*(maybeAsset)).name());
		if (!Assert(this, tTypeName == tTypeName, std::format("Tried to get asset of type:{} "
			"by name:{} MUTABLE but asset type is actually:{}", tTypeName, name, realTypename)))
			return nullptr;

		try
		{
			return dynamic_cast<T*>(maybeAsset);
		}
		catch (const std::exception& e)
		{
			LogError(this, std::format("Tried to get asset of type:{} by name:{} "
				"but an asset by that name could not be converted to the type. Error:{}", tTypeName, maybeAsset.GetName(), e.what()));
			return nullptr;
		}
		return nullptr;
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
	std::vector<T*> GetAssetsOfTypeMutable(const std::function<bool(const Asset&)> assetPredicate= nullptr)
	{
		std::vector<T*> assets = {};
		const std::string tTypeName = Utils::GetTypeName<T>();
		std::string assetTypeName = "";

		for (auto& asset : m_assets)
		{
			if (asset.second == nullptr) continue;
			assetTypeName = Utils::FormatTypeName(typeid(*(asset.second)).name());

			//LogError(std::format("Checking asset:{} ttype:{} current:{}", asset.second->ToString(), tTypeName, assetTypeName));
			if (tTypeName == assetTypeName && (assetPredicate==nullptr || assetPredicate(*(asset.second))))
			{
				try
				{
					assets.push_back(dynamic_cast<T*>(asset.second));
				}
				catch (const std::exception& e)
				{
					LogError(this, std::format("Tried to get assets of type:{} but asset at path:{} "
						"could not be converted to this type", tTypeName, asset.second->GetPath().string()));
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
				std::string assetpath = asset.GetPath().string();
				//LogError(std::format("Checking path of asset: {} to {}", asset.ToString(), targetAssetPath));
				if (assetpath.size() < targetAssetPath.size()) return false;
				return assetpath.substr(0, targetAssetPath.size()) == targetAssetPath;
			});
	}

	template<typename T, typename ...Args>
	requires IsAssetType<T> && std::is_base_of_v<IDependable<Args...>, T>
	void InitDependencoes(std::add_lvalue_reference_t<Args>... args)
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

