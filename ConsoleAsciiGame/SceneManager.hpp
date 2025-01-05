#pragma once
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <optional>
#include <string>
#include "Scene.hpp"
#include "Entity.hpp"
#include "EntityMapper.hpp"
#include "TransformData.hpp"
#include "GlobalEntityManager.hpp"

namespace SceneManagement
{
	class SceneManager
	{
	private:
		Scene* m_activeScene;
		std::vector<Scene> m_allScenes;

		/*EntityMapper m_globalEntityMapper;
		std::vector<ECS::Entity> m_globalEntities;
		EntityCollection m_globalEntitiesLookup;*/
		
	public:
		GlobalEntityManager m_GlobalEntityManager;

	private:
		Scene* TryGetSceneWithName(const std::string& sceneName);
		Scene* TryGetSceneWithIndex(const int& sceneIndex);

		void SetActiveScene(Scene* activeScene);

	public:
		SceneManager(const std::filesystem::path& allScenesDir);

		Scene* GetActiveSceneMutable();
		const Scene* GetActiveScene() const;

		bool TrySetActiveScene(const std::string& sceneName);
		bool TrySetActiveScene(const int& sceneIndex);
	};
}

