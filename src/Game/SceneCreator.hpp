#pragma once

namespace Engine::Core { class EngineState; }
namespace Engine::Scenes
{
	class Scene;
	namespace SceneCreator
	{
		void Init(Core::EngineState&);
		void OnSceneAssetLoad(Scene* scene);
		void OnActiveSceneChange(Scene* scene);
	}
}
