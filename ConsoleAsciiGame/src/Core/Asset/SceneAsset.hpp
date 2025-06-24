#pragma once
#include <optional>
#include "Core/Asset/Asset.hpp"
#include "Core/Scene/Scene.hpp"
#include "Core/Serialization/IJsonSerializable.hpp"
#include "Core/Asset/IDependableAsset.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"

namespace AssetManagement
{
	class AssetManager;
}

class SceneAsset : public Asset, public IDependableAsset<GlobalEntityManager, AssetManagement::AssetManager>
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

