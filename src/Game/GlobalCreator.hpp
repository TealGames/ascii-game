#pragma once

class GlobalEntityManager;
class CameraController;
namespace SceneManagement
{
	class SceneManager;
}
namespace AssetManagement
{
	class AssetManager;
}

namespace GlobalEntityCreator
{
	void CreateGlobals(GlobalEntityManager& manager, SceneManagement::SceneManager& sceneManager, 
		CameraController& cameraController, AssetManagement::AssetManager& assetManager);
}

