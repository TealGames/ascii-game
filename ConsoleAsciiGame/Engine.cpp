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
#include "AnimatorSystem.hpp"
#include "SpriteAnimatorSystem.hpp"
#include "CartesianPosition.hpp"
#include "Array2DPosition.hpp"
#include "Timer.hpp"
#include "ProfilerTimer.hpp"

namespace Core
{
	//-------------------------------------------------------------------
	// GLOBAL TODO CHECKLIST
	//-------------------------------------------------------------------
	//TODO: maybe make an array version for text buffer (actually this time)
	//TODO: make a separate TextArray or FragmentedTextArray (textcharpositions) as the base for text buffer and sprite
	//so they can use the same methods and checking (so it is likely we need to make it array now to improve performance)
	//TODO: for performance make all members, function params that are const std::string that do not need to be vars (like are just direct times) as const char* (const char[])

	static const std::string SCENES_PATH = "scenes";
	static constexpr std::uint8_t TARGET_FPS = 60;

	constexpr std::uint8_t NO_FRAME_LIMIT = 0;
	constexpr std::uint8_t FRAME_LIMIT = NO_FRAME_LIMIT;
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
		m_spriteAnimatorSystem(m_entityRendererSystem),
		m_animatorSystem(*this),
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

		PlayerData& playerData = playerEntity.AddComponent<PlayerData>(PlayerData{});
		LightSourceData& lightSource= playerEntity.AddComponent<LightSourceData>(LightSourceData{ 8, RenderLayerType::Background,
			ColorGradient(Color(243, 208, 67, 255), Color(228, 8, 10, 255)), std::uint8_t(254), 1.2f });

		playerEntity.AddComponent<EntityRendererData>(EntityRendererData{ VisualData{1, 1, {TextChar(GRAY, 'H')}}, RenderLayerType::Player });
		/*playerEntity.AddComponent<AnimatorData>(AnimatorData(std::vector<AnimationPropertyVariant>{
				AnimationProperty<std::uint8_t>(lightSource.m_LightRadius, lightSource.m_MutatedThisFrame, {
				AnimationPropertyKeyframe<std::uint8_t>(std::uint8_t(8), 0),
				AnimationPropertyKeyframe<std::uint8_t>(std::uint8_t(1), 1)})}, 1, 1, true));*/
		playerEntity.AddComponent<SpriteAnimatorData>(SpriteAnimatorData(
			{ SpriteAnimationFrame(0, VisualData{1, 1, TextChar(WHITE, 'O')} ),
			  SpriteAnimationFrame(2, VisualData{1, 1, TextChar(WHITE, '4')}) }, 1, 4, true));

		m_playerInfo = ECS::EntityComponentPair<PlayerData>{ playerEntity, playerData};
		
		ECS::Entity& mainCameraEntity = m_sceneManager.m_GlobalEntityManager.CreateGlobalEntity("MainCamera", TransformData({ 0, 0 }));

		CameraData& cameraData = mainCameraEntity.AddComponent<CameraData>(CameraData{ CameraSettings{playerEntity, Utils::Point2DInt(10, 10)} });

		m_sceneManager.GetActiveSceneMutable()->SetMainCamera(mainCameraEntity);
		m_mainCameraInfo = ECS::EntityComponentPair<CameraData>{ mainCameraEntity, cameraData };

		std::cout << "\n\n[ENGINE]: INITIALIZED GLOBALS" << std::endl;
		std::cout << std::format("[ENGINE]: LOADED LAYERS FOR SCENE '{}': {}", 
			m_sceneManager.GetActiveScene()->m_SceneName,
			m_sceneManager.GetActiveScene()->ToStringLayers()) << std::endl;
	}

	Engine::~Engine()
	{
	}

	LoopCode Engine::Update()
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("Engine::Update");
#endif 

		m_currentTime = std::chrono::high_resolution_clock().now();
		m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(m_currentTime - m_lastTime).count() / static_cast<float>(1000);
		m_currentFPS = 1 / m_deltaTime;

		if (FRAME_LIMIT != -1 || SHOW_FPS)
		{
			std::cout<<std::format("[ENGINE]: FRAME: {}/{} DELTA_TIME: {} FPS:{} GraphicsFPS:{}\n--------------------------------------------\n",
				std::to_string(m_currentFrameCounter + 1), std::to_string(FRAME_LIMIT), 
				Utils::ToStringDouble(m_deltaTime, DOUBLE_LOG_PRECISION), 
				Utils::ToStringDouble(m_currentFPS, DOUBLE_LOG_PRECISION), std::to_string(GetFPS()));
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

		//TODO: ideally the systems would be supplied with only relevenat components without the need of entities
		//but this can only be the case if data is stored directyl without std::any and linear component data for same entities is used

		m_transformSystem.SystemUpdate(*activeScene, m_deltaTime);
		m_playerSystem.SystemUpdate(*activeScene, *(m_playerInfo.value().m_Data), *(m_playerInfo.value().m_Entity), m_deltaTime);
		m_animatorSystem.SystemUpdate(*activeScene, m_deltaTime);
		m_spriteAnimatorSystem.SystemUpdate(*activeScene, m_deltaTime);
		m_entityRendererSystem.SystemUpdate(*activeScene, m_deltaTime);

		//TODO: light system without any other problems drop frames to ~20 fps
		m_lightSystem.SystemUpdate(*activeScene, m_deltaTime);

		m_cameraSystem.SystemUpdate(*activeScene, *mainCamera, *mainCameraEntity, m_deltaTime);
		const TextBuffer* collapsedBuffer = m_cameraSystem.GetCurrentFrameBuffer();
		Utils::Assert(this, collapsedBuffer != nullptr, std::format("Tried to render buffer from camera output, but it points to NULL data"));

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
			if (IsKeyPressed(TOGGLE_PAUSE_UPDATE))
			{
				std::cin.get();
				Utils::ClearSTDCIN();
			}

			currentCode = Update();
#ifdef ENABLE_PROFILER
			ProfilerTimer::m_Profiler.LogPerformanceTimes();
#endif

			//We do not terminate update if we want to play a few frames
			if (FRAME_LIMIT != NO_FRAME_LIMIT && m_currentFrameCounter < FRAME_LIMIT) continue;

			if (!Utils::Assert(this, currentCode != ERROR_CODE, 
				std::format("Update loop terminated due to error"))) return;

			if (!Utils::Assert(this, currentCode != END_CODE,
				std::format("Update loop terminated due to loop end triggered"))) return;
		}
	}
}
