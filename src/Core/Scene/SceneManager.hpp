#pragma once
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <optional>
#include <string>
#include "Core/Asset/SceneAsset.hpp"
#include "Core/Scene/Scene.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"
#include "Utils/Data/Event.hpp"

namespace Engine::Assets { class AssetManager; }
namespace Engine::Scenes
{
	class SceneManager
	{
	private:
		static const std::filesystem::path SCENES_FOLDER;

		Assets::AssetManager* m_assetManager;
		SceneAsset* m_activeSceneAsset;
		//TODO: sicne we may reach a poitner with many scenes, maybe we should make this a map with scene names
		std::vector<SceneAsset*> m_allScenes;
		std::filesystem::path m_allScenePath;

		/*EntityMapper m_globalEntityMapper;
		std::vector<ECS::Entity> m_globalEntities;
		EntityCollection m_globalEntitiesLookup;*/
		
	public:
		GlobalEntityManager m_GlobalEntityManager;

		Event<void, Scene*> m_OnSceneAssetLoad;
		Event<void, Scene*> m_OnActiveSceneChange;

	private:
		SceneAsset* TryGetSceneAssetMutable(const std::string& sceneName);
		SceneAsset* TryGetSceneAssetMutable(const size_t& index);

		void SetActiveScene(SceneAsset& activeScene);

	public:
		SceneManager(Assets::AssetManager& assetManager);
		~SceneManager();

		/// <summary>
		/// Although this could be handled with RAII (constructor) but 
		/// we may need to have it initialized with data after raylib window
		/// such as fonts
		/// </summary>
		void LoadAllSceneAssets();
		void SaveCurrentScene();

		int GetSceneCount() const;

		Scene* GetActiveSceneMutable();
		const Scene* GetActiveScene() const;

		bool TrySetActiveScene(const std::string& sceneName);
		bool TrySetActiveScene(const size_t& sceneIndex);

		Scene* TryGetSceneMutable(const std::string& sceneName);
		const Scene* TryGetScene(const std::string& sceneName) const;
		Scene* TryGetSceneMutable(const size_t& sceneIndex);

		const ECS::EntityData* TryGetEntity(const std::string& sceneName, const std::string& entityName) const;
		ECS::EntityData* TryGetEntityMutable(const std::string& sceneName, const std::string& entityName);

		bool ValidateAllScenes();
	};
}

