#pragma once

class GlobalEntityManager;
class CameraController;
namespace SceneManagement
{
	class SceneManager;
}

namespace GlobalCreator
{
	void CreateGlobals(GlobalEntityManager& manager, 
		SceneManagement::SceneManager& sceneManager, CameraController& cameraController);
}

