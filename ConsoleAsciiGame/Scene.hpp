#pragma once
#include <vector>
#include <queue>
#include <functional>
#include <string>
#include <filesystem>
#include <fstream>
#include "Point2DInt.hpp"
#include "TextBuffer.hpp"
#include "RenderLayer.hpp"
#include "Updateable.hpp"
#include "Entity.hpp"

class Scene : public Updateable
{
private:
	std::string m_sceneName;
	std::vector<RenderLayer> m_Layers;
	std::vector<ECS::Entity*> m_entities;
	int m_currentFrameDirtyEntities;

public:
	const std::string& m_SceneName;
	//const std::vector<RenderLayer>& m_Layers;

	const std::filesystem::path m_ScenePath;
	//The size in WIDTH, HEIGHT
	const Utils::Point2DInt m_BoundingSize;

	static const std::string m_SCENE_FILE_PREFIX;

private:
	void ParseSceneFile(std::ifstream& stream, std::vector<std::vector<TextCharPosition>>& charPos) const;
	void SortEntitiesByUpdatePriority();

public:
	Scene(const std::filesystem::path& scenePath);
	std::vector<RenderLayer*> GetLayersMutable();
	std::vector<const RenderLayer*> GetLayers() const;

	void AddEntity(ECS::Entity& entity);

	void Start() override;
	void UpdateStart(float deltaTime) override;
	void UpdateEnd(float deltaTime) override;

	std::string ToStringLayers() const;
	int GetDirtyEntitiesCount() const;
	bool HasDirtyEntities() const;
};

