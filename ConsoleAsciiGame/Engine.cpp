#include "pch.hpp"
#include "Engine.hpp"
#include "SceneManager.hpp"
#include "raylib.h"
#include "Globals.hpp"
#include "HelperFunctions.hpp"
#include "TransformSystem.hpp"
#include "EntityRendererSystem.hpp"
#include "CameraSystem.hpp"
#include "LightSourceSystem.hpp"
#include "PlayerSystem.hpp"

namespace Core
{
	static const std::string SCENES_PATH = "scenes";
	static constexpr std::uint8_t TARGET_FPS = 60;

	constexpr std::uint8_t FRAME_LIMIT = 10;
	constexpr bool SHOW_FPS = true;

	constexpr LoopCode SUCCESS_CODE = 0;
	constexpr LoopCode END_CODE = 1;
	constexpr LoopCode ERROR_CODE = 2;

	Engine::PlayerInfo::PlayerInfo(ECS::Entity& entity, PlayerData& playerData) :
		m_playerEntity(&entity), m_playerData(&playerData) {}

	void Engine::Init()
	{
		InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_NAME.c_str());
		SetTargetFPS(TARGET_FPS);
	}

	void Engine::Destroy()
	{
		CloseWindow();
	}

	Engine::Engine() : 
		m_sceneManager(SCENES_PATH),
		m_transformSystem(), 
		m_cameraSystem(m_transformSystem),
		m_entityRendererSystem(m_transformSystem),
		m_lightSystem(m_transformSystem, m_entityRendererSystem),
		m_playerSystem(m_transformSystem),
		m_playerInfo(std::nullopt),
		m_currentFrameCounter(0)
	{
		Init();
		if (!Utils::Assert(this, m_sceneManager.TrySetActiveScene(0), 
			std::format("Tried to set the active scene to the first one, but failed!"))) 
			return;

		ECS::Entity& playerEntity = m_sceneManager.m_GlobalEntityManager.CreateGlobalEntity("player", TransformData({ 0, 0 }));
		PlayerData* playerData = playerEntity.TryAddComponent<PlayerData>(PlayerData{});

		if (!Utils::Assert(this, playerData!=nullptr, std::format("Tried to create player but failed to add player data"))) 
			return;

		m_playerInfo = PlayerInfo{ playerEntity, *playerData};
	}

	Engine::~Engine()
	{
	}

	LoopCode Engine::Update()
	{
		Scene* activeScene = m_sceneManager.GetActiveSceneMutable();
		if (!Utils::Assert(this, activeScene != nullptr, std::format("Tried to update the active scene but there "
			"are none set as active right now", activeScene->m_SceneName)))
			return ERROR_CODE;

		if (!Utils::Assert(this, activeScene->HasMainCamera(), std::format("Tried to update the active scene: {}, "
			"but it has no main camera", activeScene->m_SceneName)))
			return ERROR_CODE;

		m_currentTime = std::chrono::high_resolution_clock().now();
		m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(m_currentTime - m_lastTime).count() / static_cast<float>(1000);

		if (!Utils::Assert(this, activeScene->HasEntities(), std::format("Tried to update the active scene:{} but there "
			"are no entities in the scene", activeScene->m_SceneName)))
			return ERROR_CODE;
		
		CameraData* mainCamera = activeScene->TryGetMainCameraData();
		ECS::Entity* mainCameraEntity = activeScene->TryGetMainCameraEntity();

		if (!Utils::Assert(this, mainCamera != nullptr && mainCameraEntity != nullptr,
			std::format("Tried to update the active scene:{} but failed to retrieve "
				"main camera(found:{}) and/or its entity(found:{})", activeScene->m_SceneName,
				std::to_string(mainCamera != nullptr), std::to_string(mainCameraEntity != nullptr))))
			return ERROR_CODE;

		if (!Utils::Assert(this, m_playerInfo.has_value(),
			std::format("Tried to update the active scene:{} but failed to get "
				"player info in a valid state", activeScene->m_SceneName)))
			return ERROR_CODE;

		if (FRAME_LIMIT != -1 || SHOW_FPS)
		{
			Utils::Log(std::format("FRAME: {}/{} FPS:{}\n--------------------------------------------\n",
				std::to_string(m_currentFrameCounter + 1), std::to_string(FRAME_LIMIT), std::to_string(GetFPS())));
		}

		//We need to reset to default since previous changes were baked into the buffer
		//so we need to clear it for a fresh update
		for (auto layer : activeScene->GetLayersMutable())
		{
			//Utils::Log(std::format("Resetting to  default layer: {}/{}", std::to_string(i), std::to_string(m_Layers.size()-1)));
			layer->ResetToDefault();
		}

		m_transformSystem.SystemUpdate(*activeScene, m_deltaTime);
		m_playerSystem.SystemUpdate(*activeScene, *(m_playerInfo.value().m_playerData), *(m_playerInfo.value().m_playerEntity), m_deltaTime);
		m_entityRendererSystem.SystemUpdate(*activeScene, m_deltaTime);
		m_lightSystem.SystemUpdate(*activeScene, m_deltaTime);
		m_cameraSystem.SystemUpdate(*activeScene, *mainCamera, *mainCameraEntity, m_deltaTime);

		m_transformSystem.UpdateLastFramePos(*activeScene);

		if (FRAME_LIMIT == -1) return SUCCESS_CODE;

		m_currentFrameCounter++;
		if (m_currentFrameCounter >= FRAME_LIMIT)
			return END_CODE;
	}

	void Engine::BeginUpdateLoop()
	{
		LoopCode currentCode = SUCCESS_CODE;
		while (!WindowShouldClose())
		{
			currentCode = Update();
			if (!Utils::Assert(this, currentCode != ERROR_CODE, 
				std::format("Update loop terminated due to error"))) return;

			if (!Utils::Assert(this, currentCode != END_CODE,
				std::format("Update loop terminated due to loop end triggered"))) return;
		}
	}
}
