#include <filesystem>
#include "pch.hpp"
#include "SceneManager.hpp"
#include "Scene.hpp"
#include "HelperFunctions.hpp"

namespace SceneManagement
{
	const std::filesystem::path SceneManager::SCENES_FOLDER = "scenes";

	static constexpr bool DO_SCENE_SAVING = true;
	//If empty then will write to original scene. Otherwise will write to this path,
	//which can be helpful for testing
	static const std::filesystem::path SCENE_SAVE_OUTPUT_PATH = "assets/fart.json";

	SceneManager::SceneManager(AssetManager& assetmanager) :
		m_assetManager(assetmanager), m_allScenes{}, m_activeScene(nullptr), m_GlobalEntityManager(),
		m_OnLoad(), m_OnSceneChange()
		/*m_globalEntities{}, m_globalEntitiesLookup{}, m_globalEntityMapper()*/
	{
		
	}
	SceneManager::~SceneManager()
	{
		SaveCurrentScene();
	}

	void SceneManager::SaveCurrentScene()
	{
		if (m_activeScene == nullptr || !DO_SCENE_SAVING) return;

		if (SCENE_SAVE_OUTPUT_PATH.empty()) m_activeScene->SerializeToSelf();
		else m_activeScene->SerializeToPath(SCENE_SAVE_OUTPUT_PATH);
	}

	void SceneManager::LoadAllScenes()
	{
		auto sceneAssets = m_assetManager.GetAssetsOfTypeMutable<SceneAsset>(SCENES_FOLDER);
		if (!Assert(this, sceneAssets.size() > 0, std::format("Tried to laod all scenes in scene manager "
			"but could not find any scenes at path: '{}'", SCENES_FOLDER.string())))
			return;

		//Note: by this point the asset should be valid and can be used in any way that we like
		for (auto& sceneAsset : sceneAssets)
		{
			//fileName = file.path().filename().string();
			//if (!file.is_regular_file() || fileName.size() < Scene::SCENE_FILE_PREFIX.size()) continue;
			//if (fileName.substr(0, Scene::SCENE_FILE_PREFIX.size()) != Scene::SCENE_FILE_PREFIX) continue;

			//const Scene scene = Scene();

			m_allScenes.push_back(sceneAsset);
		}

		//We need to load the scenes only after they are added in order for deserialization
		//to be able to find the scene when retrieving it from scene manager
		for (auto& scene : m_allScenes)
		{
			scene->Load();
			m_OnLoad.Invoke(&(scene->GetSceneMutable()));
			Log(std::format("Loaded scene: {}", scene->GetName()));
		}
		//LogError("Finsihed scene manager");

		/*		std::string fileName = "";
		try
		{
			for (const auto& file : std::filesystem::directory_iterator(m_allScenePath))
			{
				fileName = file.path().filename().string();
				if (!file.is_regular_file() || fileName.size() < Scene::SCENE_FILE_PREFIX.size()) continue;
				if (fileName.substr(0, Scene::SCENE_FILE_PREFIX.size()) != Scene::SCENE_FILE_PREFIX) continue;

				//const Scene scene = Scene();
				
				m_allScenes.emplace_back(file.path());
				m_allScenes.back()->GetSceneMutable().SetGlobalEntityManager(m_GlobalEntityManager);
				//LogError(std::format("Added scene:{}", m_allScenes.back().GetName()));
				//Log(std::format("Adding scene to scene manager constricutor: {}", scene.ToStringLayers()));
			}

			//We first want to make sure all scenes are found, then we load the data from the json at their respective path
			//NOTE: this is mainly to ensure that we can access all scenes when we are loading any scene (even if it has no data)
			//since there may be some scenes that have dependencies
		
			for (auto& scene : m_allScenes)
			{
				scene->Load();
				m_OnLoad.Invoke(&(scene.GetSceneMutable()));
				Log(std::format("Loaded scene: {}", scene.GetName()));
			}
		}
		catch (const std::exception& e)
		{
			Assert(this, false, std::format("Tried to get all scenes at path: {} "
				"but ran into error: {}", m_allScenePath.string(), e.what()));
		}
		*/
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

	void SceneManager::SetActiveScene(SceneAsset& activeScene)
	{
		//TODO: this should unload the old active scene and then load the new one
		//to allow for better memory usage and not having all of scenes loaded at once
		//if (m_activeScene != nullptr) m_activeScene->Unload();

		//We save the past scene if there was one
		SaveCurrentScene();

		m_activeScene = &activeScene;
		//TODO: the scene change event should proably be a reference?
		m_OnSceneChange.Invoke(&(m_activeScene->GetSceneMutable()));
		//Log(std::format("Set active scene to; {}", activeScene->ToStringLayers()));
	}

	bool SceneManager::TrySetActiveScene(const std::string& sceneName)
	{
		SceneAsset* asset = TryGetSceneAssetMutable(sceneName);
		if (!Assert(asset != nullptr, std::format("Tried to load a scene with name: {} "
			"but that scene does not exist", sceneName))) return false;

		SetActiveScene(*asset);
		return true;
	}

	bool SceneManager::TrySetActiveScene(const size_t& sceneIndex)
	{
		SceneAsset* asset = TryGetSceneAssetMutable(sceneIndex);
		//LogError(std::format("Active scene: {}", scene!=nullptr? scene->ToString() : "NULL"));
		//LogError(std::format("Active scene: {}", asset != nullptr ? "SCENE" : "NULL"));

		if (!Assert(asset != nullptr, std::format("Tried to load a scene with index: {} "
			"but that scene does not exist", std::to_string(sceneIndex)))) 
			return false;

		SetActiveScene(*asset);
		return true;
	}

	Scene* SceneManager::GetActiveSceneMutable()
	{
		if (!Assert(m_activeScene != nullptr,
			"Tried to get active scene (mutable) but there is no active scene set")) 
			return nullptr;

		return &(m_activeScene->GetSceneMutable());
	}

	const Scene* SceneManager::GetActiveScene() const
	{
		/*Log(std::format("Active scene is nullptr: {} active scene: {}", 
			std::to_string(m_activeScene==nullptr), m_activeScene==nullptr? "NULL" : m_activeScene->m_SceneName));*/

		if (!Assert(m_activeScene != nullptr,
			"Tried to get active scene (immutable) but there is no active scene set")) return nullptr;
		return &(m_activeScene->GetSceneMutable());
	}

	const ECS::Entity* SceneManager::TryGetEntity(const std::string& sceneName, const std::string& entityName) const
	{
		const Scene* maybeScene = TryGetScene(sceneName);
		if (maybeScene == nullptr) return nullptr;

		return maybeScene->TryGetEntity(entityName);
	}
	ECS::Entity* SceneManager::TryGetEntityMutable(const std::string& sceneName, const std::string& entityName)
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

