#include <filesystem>
#include "pch.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "Core/EngineState.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Scene/Scene.hpp"
#include "Utils/HelperFunctions.hpp"

namespace Engine::Scenes
{
	const std::filesystem::path SceneManager::SCENES_FOLDER = "scenes";

	/// <summary>
	/// If true will save the scene when saving is invoked (application close, new scene)
	/// </summary>
	static constexpr bool DO_SCENE_SAVING = true;
	/// <summary>
	/// If true, will load all scenes from assets into storage
	/// </summary>
	static constexpr bool LOAD_SCENES_FROM_ASSETS = true;

	//If empty then will write to original scene. Otherwise will write to this path,
	//which can be helpful for testing
	static const std::filesystem::path SCENE_DIFFERENT_SAVE_PATH = "";

	SceneManager::SceneManager(Assets::AssetManager& assetManager) :
		m_assetManager(&assetManager), m_allScenes{}, m_activeSceneAsset(nullptr), m_GlobalEntityManager(),
		m_OnSceneAssetLoad(), m_OnActiveSceneChange()
		/*m_globalEntities{}, m_globalEntitiesLookup{}, m_globalEntityMapper()*/
	{
		
	}
	SceneManager::~SceneManager()
	{
		SaveCurrentScene();
	}

	void SceneManager::SaveCurrentScene()
	{
		if (m_activeSceneAsset == nullptr || !DO_SCENE_SAVING) 
			return;

		if (!SCENE_DIFFERENT_SAVE_PATH.empty())
			m_activeSceneAsset->SaveToPath(SCENE_DIFFERENT_SAVE_PATH);
		else if (::Utils::IO::IsFileEmpty(m_activeSceneAsset->GetAbsolutePath()))
			m_activeSceneAsset->SaveToSelf();
		else
			m_activeSceneAsset->SaveToPath(m_activeSceneAsset->GetAbsolutePathCopy()+=" COPY");
	}

	void SceneManager::LoadAllSceneAssets()
	{
		if (!LOAD_SCENES_FROM_ASSETS) 
			return;

		auto sceneAssets = m_assetManager->GetAssetsOfTypeMutable<SceneAsset>(SCENES_FOLDER);
		if (sceneAssets.size() <= 0)
		{
			LogError(std::format("Tried to load all scenes in scene manager "
				"but could not find any scenes at path: '{}'", SCENES_FOLDER.string()));
			return;
		}

		//Note: by this point the asset should be valid and can be used in any way that we like
		for (auto& sceneAsset : sceneAssets)
		{
			m_allScenes.push_back(sceneAsset);
		}

		//We need to load the scenes only after they are added in order for deserialization
		//to be able to find the scene when retrieving it from scene manager
		for (auto& scene : m_allScenes)
		{
			scene->UpdateAssetFromFile();
			m_OnSceneAssetLoad.Invoke(&(scene->GetSceneMutable()));
			//Log(std::format("Loaded scene: {}", scene->GetName()));
		}
	}

	int SceneManager::GetSceneCount() const
	{
		return m_allScenes.size();
	}

	Scene* SceneManager::TryGetSceneMutable(const std::string& sceneName)
	{
		SceneAsset* assetPtr = TryGetSceneAssetMutable(sceneName);
		if (assetPtr == nullptr) return nullptr;

		return &(assetPtr->GetSceneMutable());
	}

	const Scene* SceneManager::TryGetScene(const std::string& sceneName) const
	{
		for (const auto& scene : m_allScenes)
		{
			if (scene->GetName() == sceneName)
				return &(scene->GetScene());
		}
		return nullptr;
	}

	Scene* SceneManager::TryGetSceneMutable(const size_t& sceneIndex)
	{
		SceneAsset* assetPtr = TryGetSceneAssetMutable(sceneIndex);
		if (assetPtr == nullptr) return nullptr;

		return &(assetPtr->GetSceneMutable());
	}

	SceneAsset* SceneManager::TryGetSceneAssetMutable(const std::string& sceneName)
	{
		for (const auto& scene : m_allScenes)
		{
			if (scene->GetName() == sceneName)
				return scene;
		}
		return nullptr;
	}
	SceneAsset* SceneManager::TryGetSceneAssetMutable(const size_t& sceneIndex)
	{
		if (sceneIndex < 0 || sceneIndex >= m_allScenes.size()) 
			return nullptr;

		return m_allScenes[sceneIndex];
	}

	void SceneManager::SetActiveScene(SceneAsset& activeSceneAsset)
	{
		if (m_activeSceneAsset == &activeSceneAsset)
			return;

		//TODO: this should unload the old active scene and then load the new one
		//to allow for better memory usage and not having all of scenes loaded at once
		//if (m_activeScene != nullptr) m_activeScene->Unload();

		//We save the past scene if there was one
		SaveCurrentScene();
		m_activeSceneAsset = &activeSceneAsset;
		Scene& activeScene = m_activeSceneAsset->GetSceneMutable();

		m_OnActiveSceneChange.Invoke(&activeScene);

		activeScene.Start();
		//Log(std::format("Set active scene to; {}", activeScene->ToStringLayers()));
	}

	bool SceneManager::TrySetActiveScene(const std::string& sceneName)
	{
		SceneAsset* asset = TryGetSceneAssetMutable(sceneName);
		if (asset != nullptr)
		{
			LogError(std::format("Tried to load a scene with name: {} "
				"but that scene does not exist. Total scenes:{}", sceneName,
				std::to_string(GetSceneCount())));
			return false;
		}

		SetActiveScene(*asset);
		return true;
	}

	bool SceneManager::TrySetActiveScene(const size_t& sceneIndex)
	{
		SceneAsset* asset = TryGetSceneAssetMutable(sceneIndex);
		//LogError(std::format("Active scene: {}", scene!=nullptr? scene->ToString() : "NULL"));
		//LogError(std::format("Active scene: {}", asset != nullptr ? "SCENE" : "NULL"));

		if (asset == nullptr)
		{
			LogError(std::format("Tried to load a scene with index: {} "
				"but that scene does not exist. Total Scenes:{}",
				std::to_string(sceneIndex), std::to_string(GetSceneCount())));
			return false;
		}

		SetActiveScene(*asset);
		return true; 
	}

	Scene* SceneManager::GetActiveSceneMutable()
	{
		if (!Assert(m_activeSceneAsset != nullptr,
			"Tried to get active scene (mutable) but there is no active scene set")) 
			return nullptr;

		return &(m_activeSceneAsset->GetSceneMutable());
	}

	const Scene* SceneManager::GetActiveScene() const
	{
		/*Log(std::format("Active scene is nullptr: {} active scene: {}", 
			std::to_string(m_activeScene==nullptr), m_activeScene==nullptr? "NULL" : m_activeScene->m_SceneName));*/

		if (!Assert(m_activeSceneAsset != nullptr,
			"Tried to get active scene (immutable) but there is no active scene set")) return nullptr;
		return &(m_activeSceneAsset->GetSceneMutable());
	}

	const ECS::EntityData* SceneManager::TryGetEntity(const std::string& sceneName, const std::string& entityName) const
	{
		const Scene* maybeScene = TryGetScene(sceneName);
		if (maybeScene == nullptr) return nullptr;

		return maybeScene->TryGetEntity(entityName);
	}
	ECS::EntityData* SceneManager::TryGetEntityMutable(const std::string& sceneName, const std::string& entityName)
	{
		Scene* maybeScene = TryGetSceneMutable(sceneName);
		if (maybeScene == nullptr) return nullptr;

		return maybeScene->TryGetEntityMutable(entityName);
	}

	bool SceneManager::ValidateAllScenes()
	{
		bool passesValidation = true;
		for (auto& scene : m_allScenes)
		{
			if (!scene->GetSceneMutable().Validate()) 
				passesValidation = false;
		}
		return passesValidation;
	}
}

