#pragma once
#include <optional>
#include "Asset.hpp"
#include "Scene.hpp"
#include "IJsonSerializable.hpp"
#include "IDependable.hpp"
#include "GlobalEntityManager.hpp"

class SceneAsset : public Asset, public IJsonSerializable, public ILoadable, public IDependable<GlobalEntityManager>
{
private:
	std::optional<Scene> m_scene;
public:
	static const std::string EXTENSION;

private:
public:
	SceneAsset(const std::filesystem::path& path);

	void SetDependencies(GlobalEntityManager& globalManager) override;

	Scene& GetSceneMutable();
	const Scene& GetScene() const;

	void Deserialize(const Json& json) override;
	Json Serialize() override;

	void Load() override;
	void Unload() override;
};

