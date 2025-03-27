#pragma once
#include <chrono>
#include <optional>
#include <format>
#include <string>
#include <cstdint>
#include <type_traits>
#include "SceneManager.hpp"
#include "TransformSystem.hpp"
#include "EntityRendererSystem.hpp"
#include "CameraSystem.hpp"
#include "LightSourceSystem.hpp"
#include "AnimatorSystem.hpp"
#include "SpriteAnimatorSystem.hpp"
#include "PhysicsBodySystem.hpp"
#include "PhysicsManager.hpp"
#include "AssetManager.hpp"
#include "InputSystem.hpp"
#include "InputManager.hpp"
#include "PlayerSystem.hpp"
#include "UIObjectSystem.hpp"
#include "DebugInfo.hpp"
#include "CommandConsole.hpp"
#include "CameraController.hpp"
//#include "InputManager.hpp"
#include "Debug.hpp"
#include "EntityEditorGUI.hpp"
#include "GUISelectorManager.hpp"
#include "JsonSerializers.hpp"

namespace Core
{
	using LoopCode = std::uint8_t;
	class Engine
	{
	private:

		AssetManager m_assetManager;
		SceneManagement::SceneManager m_sceneManager;
		Physics::PhysicsManager m_physicsManager;
		CameraController m_cameraController;
		Input::InputManager m_inputManager;
		GUISelectorManager m_guiSelectorManager;

		std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTime;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_currentTime;
		double m_deltaTime = 0;
		/// <summary>
		/// The step corresponding to one second. 
		/// =1 is default, <1 will slow down >1 speed up
		/// </summary>
		double m_timeStep = 1;
		int m_currentFPS = 0;

		//TODO: there has to be a way that does not involve us writing every possible system
		ECS::TransformSystem m_transformSystem;
		ECS::CameraSystem m_cameraSystem;
		ECS::UIObjectSystem m_uiSystem;
		ECS::LightSourceSystem m_lightSystem;
		//ECS::InputSystem m_inputSystem;
		ECS::EntityRendererSystem m_entityRendererSystem;
		ECS::AnimatorSystem m_animatorSystem;
		ECS::SpriteAnimatorSystem m_spriteAnimatorSystem;
		ECS::PhysicsBodySystem m_physicsBodySystem;
		ECS::PlayerSystem m_playerSystem;

		//std::optional<ECS::EntityComponents<PlayerData, PhysicsBodyData>> m_playerInfo;
		//std::optional<ECS::EntityComponentPair<CameraData>> m_mainCameraInfo;
		//std::optional<ECS::EntityComponentPair<PhysicsBodyData>> m_obstacleInfo;

		std::uint8_t m_currentFrameCounter = 0;

		DebugInfo m_debugInfo;
		bool m_enableDebugInfo;

		CommandConsole m_commandConsole;
		bool m_enableCommandConsole;

		EntityEditorGUI m_entityEditor;
	public:

	private:
		void InitEngine();
		void ValidateAll();
		void Destroy();

		/// <summary>
		/// Will run one update loop.
		/// Returns true if there was an error, or the loop prevented further execution
		/// Returns false if it can continue
		/// </summary>
		/// <returns></returns>
		LoopCode Update();

		void InitConsoleCommands();
		void EngineLog(const std::string& log) const;

	public:
		Engine();
		~Engine();

		void BeginUpdateLoop();

		////TODO: this should probably get abstracted out into a reflection manager that stores type sand their properties
		////and all systems would submit their properties and types/other metadata to it on init of engine
		//template<typename PropertyType>
		//PropertyType* TryGetPropertyFromSystem(ECS::Entity& entity, const ComponentType& type, const std::string& propertyName)
		//{
		//	if (type == ComponentType::LightSource)
		//	{
		//		LightSourceData* maybeData = entity.TryGetComponentMutable<LightSourceData>();
		//		if (!Assert(this, maybeData != nullptr, std::format("Tried to get property: {} from system for entity: {} and component: {} "
		//			"but it does not have that component", propertyName, entity.GetName(), ToString(type)))) return nullptr;

		//		if (propertyName == "LightRadius" && std::is_same_v<PropertyType, decltype(maybeData->m_LightRadius)>)
		//			//reinterpret cast is dangerous but we have to do it here since we know for sure if the condition of the same
		//			//type sxecutes we can be sure we can convert to this type since we cant just return normally since compiler is not sure whether types match
		//			return reinterpret_cast<PropertyType*>(&(maybeData->m_LightRadius));
		//		if (propertyName == "LightIntensity" && std::is_same_v<PropertyType, decltype(maybeData->m_Intensity)>)
		//			return reinterpret_cast<PropertyType*>(&(maybeData->m_Intensity));
		//		else
		//		{
		//			if (!Assert(this, maybeData != nullptr, std::format("Tried to get property: {} from system for entity: {} and component: {} "
		//				"but it did not match any names and/or their types with type: {}!", propertyName, 
		//				entity.GetName(), ToString(type), typeid(PropertyType).name()))) return nullptr;
		//		}
		//	}
		//	else
		//	{
		//		LogError(this, std::format("Tried to get property: {} from "
		//			"engine for entity: {} of an undefined type: {}", propertyName, entity.GetName(), ToString(type)));
		//	}
		//	return nullptr;
		//}
	};
}


