#pragma once
#include <optional>
#include "Core/Asset/Asset.hpp"
#include "Core/Scene/Scene.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"

namespace Engine::Assets { class AssetManager; }
namespace Engine::Scenes
{
	class SceneAsset : public Assets::Asset
	{
	private:
		Assets::AssetManager* m_assetManager;
		std::optional<Scene> m_scene;
		std::filesystem::path m_levelFilePath;
	public:
		static const std::array<std::string_view, 1> EXTENSIONS;
		static const std::string LEVEL_EXTENSION;

	private:
		bool TryLoadLevelBackground();
		Assets::AssetManager& GetAssetManagerMutable();

	protected:
	public:
		SceneAsset(const std::filesystem::path& path);

		void SetDependencies(Core::EngineState& state) override;

		Scene& GetSceneMutable();
		const Scene& GetScene() const;

		void UpdateAssetFromFile() override;
		void SaveToPath(const std::filesystem::path& path) override;
	};

	void SaveSceneToPath(const Scene& scene, const std::filesystem::path& path);
}
