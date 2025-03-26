#pragma once
#include <optional>
#include "Asset.hpp"
#include "Scene.hpp"
#include "IJsonSerializable.hpp"
#include "IDependable.hpp"
#include "GlobalEntityManager.hpp"

class SceneAsset : public Asset, protected IJsonSerializable, public ILoadable, public IDependable<GlobalEntityManager>
{
private:
	std::optional<Scene> m_scene;
public:
	static const std::string EXTENSION;

private:
protected:
	void Deserialize(const Json& json) override;
	Json Serialize() override;
public:
	SceneAsset(const std::filesystem::path& path);

	void SetDependencies(GlobalEntityManager& globalManager) override;

	Scene& GetSceneMutable();
	const Scene& GetScene() const;

	void SerializeToPath(const std::filesystem::path& path);
	void SerializeToSelf();

	void Load() override;
	void Unload() override;
};

