#pragma once
#include <chrono>
#include <optional>
#include <format>
#include <string>
#include <cstdint>
#include <type_traits>
#include "Core/Scene/SceneManager.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"
#include "ECS/Systems/Types/World/EntityRendererSystem.hpp"
#include "ECS/Systems/Types/World/CameraSystem.hpp"
#include "ECS/Systems/Types/World/LightSourceSystem.hpp"
#include "ECS/Systems/Types/World/AnimatorSystem.hpp"
#include "ECS/Systems/Types/World/SpriteAnimatorSystem.hpp"
#include "ECS/Systems/Types/World/PhysicsBodySystem.hpp"
#include "Core/Physics/PhysicsManager.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "ECS/Systems/Types/World/InputSystem.hpp"
#include "Core/Input/InputManager.hpp"
#include "ECS/Systems/Types/World/PlayerSystem.hpp"
#include "ECS/Systems/Types/World/TriggerSystem.hpp"
#include "ECS/Systems/Types/World/CollisionBoxSystem.hpp"
#include "ECS/Systems/Types/World/ParticleEmitterSystem.hpp"
#include "Core/UI/UISystemExecutor.hpp"
#include "Core/Camera/CameraController.hpp"
#include "Core/Collision/CollisionRegistry.hpp"
#include "Core/Time/TimeKeeper.hpp"
#include "Game/GameManager.hpp"
//#include "Core/Input/InputManager.hpp"
#include "Core/UI/UIHierarchy.hpp"
#include "Core/UI/UIInteractionManager.hpp"
#include "Editor/EngineEditor.hpp"
#include "Core/Rendering/GameRenderer.hpp"
#include "GlobalInitializer.hpp"
#include "Core/EngineState.hpp"
#include "Core/GizmoOverlay.hpp"

namespace Core
{
	using LoopStatusCode = std::uint8_t;
	class Engine
	{
	private:
		EngineState m_engineState;

		AssetManagement::AssetManager m_assetManager;
		GlobalInitializer m_globalInitializer;
		
		CollisionRegistry m_collisionRegistry;
		SceneManagement::SceneManager m_sceneManager;
		Physics::PhysicsManager m_physicsManager;
		CameraController m_cameraController;
		Input::InputManager m_inputManager;
		UIInteractionManager m_UIInteractionManager;
		UIHierarchy m_uiHierarchy;
		PopupUIManager m_popupManager;
		Rendering::Renderer m_renderer;

		//TODO: there has to be a way that does not involve us writing every possible system
		ECS::TransformSystem m_transformSystem;
		ECS::CameraSystem m_cameraSystem;
		ECS::LightSourceSystem m_lightSystem;
		//ECS::InputSystem m_inputSystem;
		ECS::EntityRendererSystem m_entityRendererSystem;
		ECS::AnimatorSystem m_animatorSystem;
		ECS::SpriteAnimatorSystem m_spriteAnimatorSystem;
		ECS::CollisionBoxSystem m_collisionBoxSystem;
		ECS::PhysicsBodySystem m_physicsBodySystem;
		ECS::PlayerSystem m_playerSystem;
		ECS::ParticleEmitterSystem m_particleEmitterSystem;
		ECS::TriggerSystem m_triggerSystem;

		UISystemExecutor m_uiSystemExecutor;
		GizmoOverlay m_gizmosOverlay;

		//std::optional<ECS::EntityComponents<PlayerData, PhysicsBodyData>> m_playerInfo;
		//std::optional<ECS::EntityComponentPair<CameraData>> m_mainCameraInfo;
		//std::optional<ECS::EntityComponentPair<PhysicsBodyData>> m_obstacleInfo;

		//std::uint8_t m_currentFrameCounter = 0;

		TimeKeeper m_timeKeeper;
		EngineEditor m_editor;
		GameManager m_gameManager;
	public:

	private:
		void ValidateAll();
		void StartAll();
		void Destroy();

		/// <summary>
		/// Will run one update loop.
		/// Returns true if there was an error, or the loop prevented further execution
		/// Returns false if it can continue
		/// </summary>
		/// <returns></returns>
		LoopStatusCode Update();

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
		//		if (!Assert(maybeData != nullptr, std::format("Tried to get property: {} from system for entity: {} and component: {} "
		//			"but it does not have that component", propertyName, entity.GetName(), ToString(type)))) return nullptr;

		//		if (propertyName == "LightRadius" && std::is_same_v<PropertyType, decltype(maybeData->m_LightRadius)>)
		//			//reinterpret cast is dangerous but we have to do it here since we know for sure if the condition of the same
		//			//type sxecutes we can be sure we can convert to this type since we cant just return normally since compiler is not sure whether types match
		//			return reinterpret_cast<PropertyType*>(&(maybeData->m_LightRadius));
		//		if (propertyName == "LightIntensity" && std::is_same_v<PropertyType, decltype(maybeData->m_Intensity)>)
		//			return reinterpret_cast<PropertyType*>(&(maybeData->m_Intensity));
		//		else
		//		{
		//			if (!Assert(maybeData != nullptr, std::format("Tried to get property: {} from system for entity: {} and component: {} "
		//				"but it did not match any names and/or their types with type: {}!", propertyName, 
		//				entity.GetName(), ToString(type), typeid(PropertyType).name()))) return nullptr;
		//		}
		//	}
		//	else
		//	{
		//		LogError(std::format("Tried to get property: {} from "
		//			"engine for entity: {} of an undefined type: {}", propertyName, entity.GetName(), ToString(type)));
		//	}
		//	return nullptr;
		//}
	};
}


