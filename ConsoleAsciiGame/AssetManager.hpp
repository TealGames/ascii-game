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
	//Note: these are all heap allocated
	/// </summary>
	std::unordered_map<std::string, Asset*> m_assets;

public:

private:
public:
	AssetManager();
	~AssetManager();

	bool Validate() override;

	std::filesystem::path GetAssetPath(const std::filesystem::path& directoryFile) const;

	template<typename T>
	requires IsAssetType<T>
	bool TryAddAsset(const std::filesystem::path& assetPath)
	{
		//TODO: make sure we create an asset based on the type so we can use polymorphism
		//TODO: check for duplicates 
		Asset* newAsset = new T(assetPath);
		m_assets.emplace(newAsset->GetName(), newAsset);
		return true;
	}

	Asset* TryGetAssetMutable(const std::string& name);
	const Asset* TryGetAsset(const std::string& name) const;

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
				LogError(std::format("Checking path of asset: {} to {}", asset.ToString(), targetAssetPath));
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

