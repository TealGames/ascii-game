#pragma once
#include <chrono>
#include <format>
#include <string>
#include <cstdint>
#include "Core/Scene/SceneManager.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"
#include "ECS/Systems/Types/World/EntityRenderer2DSystem.hpp"
#include "ECS/Systems/Types/World/CameraSystem.hpp"
#include "ECS/Systems/Types/World/LightSource2DSystem.hpp"
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
#include "ECS/Systems/Types/World/Mesh3DSystem.hpp"
#include "Core/UI/UISystemExecutor.hpp"
#include "Core/Camera/CameraController.hpp"
#include "Core/Collision/CollisionRegistry.hpp"
#include "Core/Time/TimeKeeper.hpp"
#include "Game/GameManager.hpp"
//#include "Core/Input/InputManager.hpp"
#include "Core/UI/UIHierarchy.hpp"
#include "Core/UI/UIInteractionManager.hpp"
#include "Editor/EngineEditor.hpp"
#include "Core/Rendering/Renderer3d.hpp"
#include "GlobalInitializer.hpp"
#include "Core/EngineState.hpp"
#include "Core/GizmoOverlay.hpp"
#include "Core/Window/WindowManager.hpp"
#include "Core/Rendering/GraphicsManager.hpp"

namespace Core
{
	class Engine
	{
	private:
		WindowManager m_windowManager;
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
		Rendering::GraphicsManager m_graphicsManager;

		//TODO: there has to be a way that does not involve us writing every possible system
		ECS::TransformSystem m_transformSystem;
		ECS::CameraSystem m_cameraSystem;
		ECS::LightSource2DSystem m_lightSystem;
		//ECS::InputSystem m_inputSystem;
		ECS::EntityRenderer2DSystem m_entityRendererSystem;
		ECS::AnimatorSystem m_animatorSystem;
		ECS::SpriteAnimatorSystem m_spriteAnimatorSystem;
		ECS::Mesh3DSystem m_meshSystem;
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
		void SystemValidate();
		void SystemStart(Scene& scene);
		void Destroy();

		/// <summary>
		/// Will run one update loop.
		/// Returns true if there was an error, or the loop prevented further execution
		/// Returns false if it can continue
		/// </summary>
		/// <returns></returns>
		void SystemUpdate(Window& window);

		void SetUpdateStatusCode(const UpdateStatusCode& code);

	public:
		Engine();
		~Engine();

		void BeginUpdateLoop();
	};
}


