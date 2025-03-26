#pragma once

class GlobalEntityManager;
namespace SceneManagement
{
	class SceneManager;
}

namespace GlobalCreator
{
	void CreateGlobals(GlobalEntityManager& manager, SceneManagement::SceneManager& sceneManager);
}

