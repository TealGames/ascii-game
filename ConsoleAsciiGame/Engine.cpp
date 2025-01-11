#include "pch.hpp"
#include <thread>
#include "Engine.hpp"
#include "SceneManager.hpp"
#include "raylib.h"
#include "Globals.hpp"
#include "GameRenderer.hpp"
#include "HelperFunctions.hpp"
#include "TransformSystem.hpp"
#include "EntityRendererSystem.hpp"
#include "CameraSystem.hpp"
#include "LightSourceSystem.hpp"
#include "PlayerSystem.hpp"
#include "CartesianPosition.hpp"
#include "Array2DPosition.hpp"

namespace Core
{
	//-------------------------------------------------------------------
	// GLOBAL TODO CHECKLIST
	//-------------------------------------------------------------------
	//TODO: instead of point2d for both row col and cartesian pos separate them out into different types
	//so it is less confusing and easy to understand what to convert to

	static const std::string SCENES_PATH = "scenes";
	static constexpr std::uint8_t TARGET_FPS = 60;

	constexpr std::uint8_t NO_FRAME_LIMIT = 0;
	constexpr std::uint8_t FRAME_LIMIT = 2;
	constexpr bool SHOW_FPS = true;

	constexpr std::streamsize DOUBLE_LOG_PRECISION = 8;

	const Rendering::RenderInfo RENDER_INFO = 
	{
		CHAR_AREA,
		TEXT_BUFFER_PADDING,
		Utils::Point2DInt(SCREEN_WIDTH, SCREEN_HEIGHT),
		TEXT_BUFFER_FONT,
		true
	};

	//If true, even if the current output from camera is null, will render default data.
	//This is useful for testing the render loop and finding out FPS (since fps depends on drawing loop)
	constexpr bool ALWAYS_RENDER = true;
	const TextBuffer DEFAULT_RENDER_DATA = TextBuffer(22, 1, WHITE, {
		{'N', 'O', EMPTY_CHAR_PLACEHOLDER, 'C', 'A', 'M', 'E', 'R', 'A',
		EMPTY_CHAR_PLACEHOLDER, 'O','U', 'T', 'P', 'U', 'T', EMPTY_CHAR_PLACEHOLDER,
		'F', 'O', 'U', 'N', 'D'} });

	const KeyboardKey TOGGLE_PAUSE_UPDATE = KEY_E;

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
		m_currentFPS(0),
		m_currentTime(std::chrono::high_resolution_clock().now()),
		m_lastTime(std::chrono::high_resolution_clock().now()),
		m_playerInfo(std::nullopt),
		m_mainCameraInfo(std::nullopt)
	{
		Init();
		if (!Utils::Assert(this, m_sceneManager.TrySetActiveScene(0), 
			std::format("Tried to set the active scene to the first one, but failed!"))) 
			return;

		ECS::Entity& playerEntity = m_sceneManager.m_GlobalEntityManager.CreateGlobalEntity("Player", TransformData({ 0, 0 }));
		/*Utils::Log(std::format("Created entity with name {} with id: {}",
			playerEntity.m_Name, std::to_string(playerEntity.m_Id)));*/

		PlayerData& playerData = playerEntity.AddComponent<PlayerData>(PlayerData{});
		playerEntity.AddComponent<LightSourceData>(LightSourceData{ 8, RenderLayerType::Background,
			ColorGradient(Color(243, 208, 67, 255), Color(228, 8, 10, 255)), std::uint8_t(254), 1.2f });
		/*Utils::Assert(addedLight, "Failed to add player light");*/

		playerEntity.AddComponent<EntityRendererData>(EntityRendererData{ { {TextChar(GRAY, 'H')}}, RenderLayerType::Player});
		/*Utils::Assert(addedRender, "Failed to add player renderer");*/

		/*if (!Utils::Assert(this, playerData!=nullptr, std::format("Tried to create player but failed to add player data. "
			"Player : {}", playerEntity.ToString()))) 
			return;*/

		m_playerInfo = ECS::EntityComponentPair<PlayerData>{ playerEntity, playerData};
		
		ECS::Entity& mainCameraEntity = m_sceneManager.m_GlobalEntityManager.CreateGlobalEntity("MainCamera", TransformData({ 0, 0 }));
		/*Utils::Log(std::format("Created entity with name {} with id: {} ",
			mainCameraEntity.m_Name, std::to_string(mainCameraEntity.m_Id)));*/

		CameraData& cameraData = mainCameraEntity.AddComponent<CameraData>(CameraData{ CameraSettings{playerEntity, Utils::Point2DInt(10, 10)} });
	/*	if (!Utils::Assert(this, cameraData != nullptr, std::format("Tried to create main camera but failed to add camera data. "
			"Main Camera : {}", mainCameraEntity.ToString())))
			return;*/

		/*Utils::Log(std::format("All scene data. Player id: {} camera id: {}; {}",
			std::to_string(playerEntity.m_Id), std::to_string(mainCameraEntity.m_Id),
			m_sceneManager.GetActiveScene()->ToStringEntityData()));
		Utils::Log(std::format("Camera data has follow is: {}", cameraData->m_CameraSettings.m_FollowTarget->ToString()));*/

		m_sceneManager.GetActiveSceneMutable()->SetMainCamera(mainCameraEntity);
		Utils::Log(std::format("Current active camera is: {}", std::to_string(m_sceneManager.GetActiveSceneMutable()->TryGetMainCameraData()!=nullptr)));
		m_mainCameraInfo = ECS::EntityComponentPair<CameraData>{ mainCameraEntity, cameraData };
	}

	Engine::~Engine()
	{
	}

	LoopCode Engine::Update()
	{
		m_currentTime = std::chrono::high_resolution_clock().now();
		m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(m_currentTime - m_lastTime).count() / static_cast<float>(1000);
		m_currentFPS = 1 / m_deltaTime;

		if (FRAME_LIMIT != -1 || SHOW_FPS)
		{
			//Utils::Log(std::format("last: {} currnet: {}", m_lastTime, ));
			Utils::Log(std::format("FRAME: {}/{} DELTA_TIME: {} FPS:{} GraphicsFPS:{}\n--------------------------------------------\n",
				std::to_string(m_currentFrameCounter + 1), std::to_string(FRAME_LIMIT), 
				Utils::ToStringDouble(m_deltaTime, DOUBLE_LOG_PRECISION), 
				Utils::ToStringDouble(m_currentFPS, DOUBLE_LOG_PRECISION), std::to_string(GetFPS())));
		}

		/*m_lastTime = m_currentTime;
		return SUCCESS_CODE;*/

		Scene* activeScene = m_sceneManager.GetActiveSceneMutable();
		if (!Utils::Assert(this, activeScene != nullptr, std::format("Tried to update the active scene but there "
			"are none set as active right now", activeScene->m_SceneName)))
			return ERROR_CODE;

		if (!Utils::Assert(this, activeScene->HasMainCamera(), std::format("Tried to update the active scene: {}, "
			"but it has no main camera", activeScene->m_SceneName)))
			return ERROR_CODE;

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

		//We need to reset to default since previous changes were baked into the buffer
		//so we need to clear it for a fresh update

		//for (auto& layer : activeScene->GetLayersMutable())
		//{
		//	//Utils::Log(std::format("Resetting to  default layer: {}/{}", std::to_string(i), std::to_string(m_Layers.size()-1)));
		//	layer->ResetToDefault();
		//}
	
		//TODO: maybe some general scene stuff should be abstracted into scene manager
		activeScene->ResetAllLayers();
		activeScene->ResetFrameDirtyComponentCount();

		Utils::Log(std::format("All render layers: {}", activeScene->ToStringLayers()));

		//TODO: ideally the systems would be supplied with only relevenat components without the need of entities
		//but this can only be the case if data is stored directyl without std::any and linear component data for same entities is used

		m_transformSystem.SystemUpdate(*activeScene, m_deltaTime);
		m_playerSystem.SystemUpdate(*activeScene, *(m_playerInfo.value().m_Data), *(m_playerInfo.value().m_Entity), m_deltaTime);
		m_entityRendererSystem.SystemUpdate(*activeScene, m_deltaTime);

		//TODO: light system without any other problems drop frames to ~20 fps
		m_lightSystem.SystemUpdate(*activeScene, m_deltaTime);

		m_cameraSystem.SystemUpdate(*activeScene, *mainCamera, *mainCameraEntity, m_deltaTime);
		const TextBuffer* collapsedBuffer = m_cameraSystem.GetCurrentFrameBuffer();
		Utils::Assert(this, collapsedBuffer != nullptr, std::format("Tried to render buffer from camera output, but it points to NULL data"));
		if (collapsedBuffer != nullptr) Utils::Log(std::format("Collapsed buffer: {}", collapsedBuffer->ToString()));

		//TODO: rendering buffer drops frames
		if (collapsedBuffer != nullptr) Rendering::RenderBuffer(*collapsedBuffer, RENDER_INFO);
		else if (ALWAYS_RENDER) Rendering::RenderBuffer(DEFAULT_RENDER_DATA, RENDER_INFO);

		m_transformSystem.UpdateLastFramePos(*activeScene);

		//Utils::Log(std::format("Player pos: {}", m_playerInfo.value().m_Entity->m_Transform.m_Pos.ToString()));
		//Utils::Log(m_sceneManager.GetActiveScene()->ToStringLayers());

		m_lastTime = m_currentTime;

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
			Utils::Log(std::format("Is key: {} down", IsKeyPressed(TOGGLE_PAUSE_UPDATE)));
			/*if (!m_isLoopRunning && m_currentUpdatePauseCooldownFrames < TOGGLE_UPDATE_COOLDONW_FRAMES)
			{
				m_currentUpdatePauseCooldownFrames++;
				continue;
			}*/

			if (IsKeyPressed(TOGGLE_PAUSE_UPDATE))
			{
				std::cin.get();
				Utils::ClearSTDCIN();
			}

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
