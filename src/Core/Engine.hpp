#pragma once
#include <chrono>
#include <format>
#include <string>
#include <cstdint>
#include "Core/Scene/SceneManager.hpp"
#include "Core/UI/PopupUIManager.hpp"
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
#include "Core/UI/UIHierarchy.hpp"
#include "Core/UI/UIInteractionManager.hpp"
#include "Editor/EngineEditor.hpp"
#include "Core/Rendering/Renderer3d.hpp"
#include "Core/EngineState.hpp"
#include "Editor/GizmoOverlay.hpp"
#include "Core/Debug/CommandController.hpp"
#include "Core/Window/WindowManager.hpp"
#include "Core/Rendering/GraphicsManager.hpp"

namespace Engine::Core
{
	class Engine
	{
	private:
		//NOTE: the order below until engine state is based on dependencies and 
		//they must all come before engine state since eninge state is constructed with them
		TimeKeeper m_timeKeeper;
		WindowManager m_windowManager;
		Assets::AssetManager m_assetManager;
		Camera::CameraController m_cameraController;
		Scenes::SceneManager m_sceneManager;
		Input::InputManager m_inputManager;
		Rendering::GraphicsManager m_graphicsManager;

		EngineState m_engineState;

		Physics::CollisionRegistry m_collisionRegistry;
		Physics::PhysicsManager m_physicsManager;
		UI::UIInteractionManager m_UIInteractionManager;
		UI::UIHierarchy m_uiHierarchy;
		UI::PopupUIManager m_popupManager;
		Rendering::Renderer m_renderer;

		//TODO: there has to be a way that does not involve us writing every possible system
		TransformSystem m_transformSystem;
		Camera::CameraSystem m_cameraSystem;
		Lighting2D::LightSource2DSystem m_lightSystem;
		Rendering::EntityRenderer2DSystem m_entityRendererSystem;
		Rendering::Mesh3DSystem m_meshSystem;
		Animation::AnimatorSystem m_animatorSystem;
		Animation::SpriteAnimatorSystem m_spriteAnimatorSystem;
		Physics::CollisionBoxSystem m_collisionBoxSystem;
		Physics::PhysicsBodySystem m_physicsBodySystem;
		Player::PlayerSystem m_playerSystem;
		ParticleSystem::ParticleEmitterSystem m_particleEmitterSystem;
		World::TriggerSystem m_triggerSystem;

		UI::UISystemExecutor m_uiSystemExecutor;
		Debug::CommandController m_commandController;
		Editor::Debug::GizmoOverlay m_gizmosOverlay;

		//std::optional<ECS::EntityDatas<PlayerComponent, PhysicsBodyData>> m_playerInfo;
		//std::optional<ECS::EntityDataPair<CameraData>> m_mainCameraInfo;
		//std::optional<ECS::EntityDataPair<PhysicsBodyData>> m_obstacleInfo;

		//std::uint8_t m_currentFrameCounter = 0;

		Editor::EngineEditor m_editor;
		GameManager m_gameManager;
	public:

	private:
		void SystemValidate();
		void SystemStart(Scenes::Scene& scene);
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


