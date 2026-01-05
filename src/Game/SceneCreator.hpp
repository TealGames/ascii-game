#pragma once

class Scene;
namespace AssetManagement { class AssetManager; }
namespace SceneCreator
{
	void OnSceneLoad(Scene& scene, AssetManagement::AssetManager& assetManager);
	void OnSceneStart(Scene& scene, AssetManagement::AssetManager& assetManager);
}