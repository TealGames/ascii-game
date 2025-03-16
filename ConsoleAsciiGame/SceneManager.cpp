#include <filesystem>
#include "pch.hpp"
#include "SceneManager.hpp"
#include "Scene.hpp"
#include "HelperFunctions.hpp"

namespace SceneManagement
{
	SceneManager::SceneManager(const std::filesystem::path& allScenesDir) : 
		m_allScenes{}, m_activeScene(nullptr), m_GlobalEntityManager(), m_allScenePath(allScenesDir), 
		m_OnLoad(), m_OnSceneChange()
		/*m_globalEntities{}, m_globalEntitiesLookup{}, m_globalEntityMapper()*/
	{
		
	}

	void SceneManager::LoadAllScenes()
	{
		std::string fileName = "";
		try
		{
			for (const auto& file : std::filesystem::directory_iterator(m_allScenePath))
			{
				fileName = file.path().filename().string();
				if (!file.is_regular_file() || fileName.size() < Scene::SCENE_FILE_PREFIX.size()) continue;
				if (fileName.substr(0, Scene::SCENE_FILE_PREFIX.size()) != Scene::SCENE_FILE_PREFIX) continue;

				//const Scene scene = Scene();
				m_allScenes.emplace_back(file.path(), m_GlobalEntityManager);
				//LogError(std::format("Added scene:{}", m_allScenes.back().GetName()));
				//Log(std::format("Adding scene to scene manager constricutor: {}", scene.ToStringLayers()));
			}

			//We first want to make sure all scenes are found, then we load the data from the json at their respective path
			//NOTE: this is mainly to ensure that we can access all scenes when we are loading any scene (even if it has no data)
			//since there may be some scenes that have dependencies
		
			for (auto& scene : m_allScenes)
			{
				scene.Load();
				m_OnLoad.Invoke(&scene);
				Log(std::format("Loaded scene: {}", scene.GetName()));
			}
		}
		catch (const std::exception& e)
		{
			Assert(this, false, std::format("Tried to get all scenes at path: {} "
				"but ran into error: {}", m_allScenePath.string(), e.what()));
		}
	}

	int SceneManager::GetSceneCount() const
	{
		return m_allScenes.size();
	}

	Scene* SceneManager::TryGetSceneWithNameMutable(const std::string& sceneName)
	{
		for (auto& scene : m_allScenes)
		{
			if (scene.GetName() == sceneName)
				return &scene;
		}
		return nullptr;
	}

	Scene* SceneManager::TryGetSceneWithIndexMutable(const int& sceneIndex)
	{
		if (sceneIndex < 0 || sceneIndex >= m_allScenes.size()) return nullptr;
		/*Log(std::format("Attempting to get valid index scene {}/{}. scene valid: {}",
			std::to_string(sceneIndex), std::to_string(m_allScenes.size()), m_allScenes[sceneIndex].m_SceneName));*/
		return &(m_allScenes[sceneIndex]);
	}

	void SceneManager::SetActiveScene(Scene* activeScene)
	{
		//TODO: this should unload the old active scene and then load the new one
		//to allow for better memory usage and not having all of scenes loaded at once
		//if (m_activeScene != nullptr) m_activeScene->Unload();
		m_activeScene = activeScene;
		m_OnSceneChange.Invoke(m_activeScene);
		//Log(std::format("Set active scene to; {}", activeScene->ToStringLayers()));
	}

	bool SceneManager::TrySetActiveScene(const std::string& sceneName)
	{
		Scene* scene = TryGetSceneWithNameMutable(sceneName);
		if (!Assert(scene != nullptr, std::format("Tried to load a scene with name: {} "
			"but that scene does not exist", sceneName))) return false;

		SetActiveScene(scene);
		return true;
	}

	bool SceneManager::TrySetActiveScene(const int& sceneIndex)
	{
		Scene* scene = TryGetSceneWithIndexMutable(sceneIndex);
		//Log(std::format("Active scene: {}", scene!=nullptr? scene->ToStringLayers() : "NULL"));

		if (!Assert(scene != nullptr, std::format("Tried to load a scene with index: {} "
			"but that scene does not exist", std::to_string(sceneIndex)))) return false;

		SetActiveScene(scene);
		return true;
	}

	Scene* SceneManager::GetActiveSceneMutable()
	{
		if (!Assert(m_activeScene != nullptr,
			"Tried to get active scene (mutable) but there is no active scene set")) return nullptr;
		return m_activeScene;
	}

	const Scene* SceneManager::GetActiveScene() const
	{
		/*Log(std::format("Active scene is nullptr: {} active scene: {}", 
			std::to_string(m_activeScene==nullptr), m_activeScene==nullptr? "NULL" : m_activeScene->m_SceneName));*/

		if (!Assert(m_activeScene != nullptr,
			"Tried to get active scene (immutable) but there is no active scene set")) return nullptr;
		return m_activeScene;
	}

	bool SceneManager::ValidateAllScenes()
	{
		bool passesValidation = true;
		for (auto& scene : m_allScenes)
		{
			if (!scene.Validate()) passesValidation = false;
		}
		return passesValidation;
	}
}

