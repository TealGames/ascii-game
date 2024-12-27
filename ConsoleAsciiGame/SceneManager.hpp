#pragma once
#include <filesystem>
#include <vector>
#include <optional>
#include <string>
#include "Scene.hpp"

namespace SceneManagement
{
	class SceneManager
	{
	private:
		Scene* m_activeScene;
		std::vector<Scene> m_allScenes;

	public:

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

