#pragma once
#include <optional>
#include "Asset.hpp"
#include "Scene.hpp"
#include "IJsonSerializable.hpp"
#include "IDependable.hpp"
#include "GlobalEntityManager.hpp"

namespace AssetManagement
{
	class AssetManager;
}

class SceneAsset : public Asset, public IDependable<GlobalEntityManager, AssetManagement::AssetManager>
{
private:
	AssetManagement::AssetManager* m_assetManager;
	std::optional<Scene> m_scene;
public:
	static const std::string EXTENSION;
	static const std::string LEVEL_EXTENSION;

private:
	bool TryLoadLevelBackground();
	AssetManagement::AssetManager& GetAssetManagerMutable();

protected:
public:
	SceneAsset(const std::filesystem::path& path);

	void SetDependencies(GlobalEntityManager& globalManager, AssetManagement::AssetManager& assetManager) override;

	Scene& GetSceneMutable();
	const Scene& GetScene() const;

	void UpdateAssetFromFile() override;
	void SaveToPath(const std::filesystem::path& path) override;
};

