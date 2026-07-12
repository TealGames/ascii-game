#pragma once

namespace Engine::Scenes
{
	class SceneManager;
	class GlobalEntityManager;
}
namespace Engine::Camera { class CameraController; }
namespace Engine::Assets { class AssetManager; }

namespace Engine::Scenes::GlobalEntityCreator
{
	void OnGlobalsInit(GlobalEntityManager& manager, Scenes::SceneManager& sceneManager, 
		Camera::CameraController& cameraController, Assets::AssetManager& assetManager);
}

