#pragma once

class Scene;
class EngineState;
namespace SceneCreator
{
	void OnSceneLoad(Scene& scene, EngineState& state);
	void OnSceneStart(Scene& scene, EngineState& state);
}