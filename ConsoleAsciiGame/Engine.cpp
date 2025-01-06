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

	constexpr std::uint8_t NO_FRAME_LIMIT = 0;
	constexpr std::uint8_t FRAME_LIMIT = NO_FRAME_LIMIT;
	constexpr bool SHOW_FPS = true;

	constexpr LoopCode SUCCESS_CODE = 0;
	constexpr LoopCode END_CODE = 1;
	constexpr LoopCode ERROR_CODE = 2;
		

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
		m_currentFrameCounter(0),
		m_playerInfo(std::nullopt),
		m_mainCameraInfo(std::nullopt)
	{
		Init();
		if (!Utils::Assert(this, m_sceneManager.TrySetActiveScene(0), 
			std::format("Tried to set the active scene to the first one, but failed!"))) 
			return;

		ECS::Entity& playerEntity = m_sceneManager.m_GlobalEntityManager.CreateGlobalEntity("Player", TransformData({ 0, 0 }));
		Utils::Log(std::format("Created entity with name {} with id: {}",
			playerEntity.m_Name, std::to_string(playerEntity.m_Id)));

		PlayerData* playerData = playerEntity.TryAddComponent<PlayerData>(PlayerData{});
		bool addedLight = playerEntity.TryAddComponent<LightSourceData>(LightSourceData{ 8, RenderLayerType::Background | RenderLayerType::Player,
			ColorGradient(Color(243, 208, 67, 255), Color(228, 8, 10, 255)), std::uint8_t(254), 1.2f });
		Utils::Assert(addedLight, "Failed to add player light");

		bool addedRender= playerEntity.TryAddComponent<EntityRendererData>(EntityRendererData{ { {TextChar(GRAY, 'H')}}, RenderLayerType::Player});
		Utils::Assert(addedRender, "Failed to add player renderer");

		if (!Utils::Assert(this, playerData!=nullptr, std::format("Tried to create player but failed to add player data. "
			"Player : {}", playerEntity.ToString()))) 
			return;

		m_playerInfo = ECS::EntityComponentPair<PlayerData>{ playerEntity, *playerData};
		
		ECS::Entity& mainCameraEntity = m_sceneManager.m_GlobalEntityManager.CreateGlobalEntity("MainCamera", TransformData({ 0, 0 }));
		Utils::Log(std::format("Created entity with name {} with id: {} ",
			mainCameraEntity.m_Name, std::to_string(mainCameraEntity.m_Id)));

		CameraData* cameraData = mainCameraEntity.TryAddComponent<CameraData>(CameraData{ CameraSettings{playerEntity, Utils::Point2DInt(10, 10)} });
		if (!Utils::Assert(this, cameraData != nullptr, std::format("Tried to create main camera but failed to add camera data. "
			"Main Camera : {}", mainCameraEntity.ToString())))
			return;

		Utils::Log(std::format("All scene data. Player id: {} camera id: {}; {}",
			std::to_string(playerEntity.m_Id), std::to_string(mainCameraEntity.m_Id),
			m_sceneManager.GetActiveScene()->ToStringEntityData()));
		Utils::Log(std::format("Camera data has follow is: {}", cameraData->m_CameraSettings.m_FollowTarget->ToString()));

		m_sceneManager.GetActiveSceneMutable()->SetMainCamera(mainCameraEntity);
		Utils::Log(std::format("Current active camera is: {}", std::to_string(m_sceneManager.GetActiveSceneMutable()->TryGetMainCameraData()!=nullptr)));
		m_mainCameraInfo = ECS::EntityComponentPair<CameraData>{ mainCameraEntity, *cameraData };
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

		//TODO: ideally the systems would be supplied with only relevenat components without the need of entities
		//but this can only be the case if data is stored directyl without std::any and linear component data for same entities is used
		m_transformSystem.SystemUpdate(*activeScene, m_deltaTime);
		m_playerSystem.SystemUpdate(*activeScene, *(m_playerInfo.value().m_Data), *(m_playerInfo.value().m_Entity), m_deltaTime);
		m_entityRendererSystem.SystemUpdate(*activeScene, m_deltaTime);
		m_lightSystem.SystemUpdate(*activeScene, m_deltaTime);

		m_cameraSystem.SystemUpdate(*activeScene, *mainCamera, *mainCameraEntity, m_deltaTime);
		m_transformSystem.UpdateLastFramePos(*activeScene);

		Utils::Log(std::format("Player pos: {}", m_playerInfo.value().m_Entity->m_Transform.m_Pos.ToString()));

		Utils::Log(m_sceneManager.GetActiveScene()->ToStringLayers());

		if (FRAME_LIMIT == NO_FRAME_LIMIT) return SUCCESS_CODE;

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
			//We do not terminate update if we want to play a few frames
			if (FRAME_LIMIT != NO_FRAME_LIMIT && m_currentFrameCounter < FRAME_LIMIT) continue;

			if (!Utils::Assert(this, currentCode != ERROR_CODE, 
				std::format("Update loop terminated due to error"))) return;

			if (!Utils::Assert(this, currentCode != END_CODE,
				std::format("Update loop terminated due to loop end triggered"))) return;
		}
	}
}
