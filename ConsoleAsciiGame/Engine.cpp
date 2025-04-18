#include "pch.hpp"
#include "Engine.hpp"
#include "SceneManager.hpp"
#include "raylib.h"
#include "Globals.hpp"
#include "Debug.hpp"
#include "GameRenderer.hpp"
#include "HelperFunctions.hpp"
#include "TransformSystem.hpp"
#include "EntityRendererSystem.hpp"
#include "CameraSystem.hpp"
#include "LightSourceSystem.hpp"
#include "InputSystem.hpp"
#include "AnimatorSystem.hpp"
#include "SpriteAnimatorSystem.hpp"
#include "PhysicsBodySystem.hpp"
#include "Array2DPosition.hpp"
#include "Timer.hpp"
#include "ProfilerTimer.hpp"
#include "PositionConversions.hpp"
#include "DebugInfo.hpp"
#include "InputProfileAsset.hpp"
#include "JsonSerializers.hpp"
#include "GlobalCreator.hpp"

namespace Core
{
	//-------------------------------------------------------------------
	// GLOBAL TODO CHECKLIST
	//-------------------------------------------------------------------
	//TODO: maybe make an array version for text buffer (actually this time)
	//TODO: for performance make all members, function params that are const std::string that do not need to be vars (like are just direct times) as const char* (const char[])
	//TODO: currentyl the camera maps world pos to its pos based on whole screen to accureately position it basewd on the viewport, but it does not manipulate the size 
	//meaning that for example a pos at the top left will maintain its distance from the top edge in the camera output but say something like size of a collider
	//will keep its size from world pos to screen pos (making it inconcistent with sizing)
	//TODO: input component is basically useless now should it be removed?
	//TODO: maybe combine all key,gamepad buttons into one enum and then separate check them based on its enum internal value within input manager
	//to not need dependence on raylib for keyboardkey and other input stuff and to keep it an implementation detail
	//TODO: guirect and aabb both require similar things and have similar features/functions/strucutre perhaps they should be merged into one general type
	//or they should both contain a more general type and extend its features
	//TODO: create a general selectable with selection rect, padding, select/deselect functions as well as a general selection profile with mouse key up,down select actions, etc
	//TODO: sizing on all gui objeccts should be relative in case screen size changes
	//TODO: bug where player on start can be moved to the end of the ground rectangle (probably due to it being the min distance when considerign x and y moves)
	//TODO: maybe remake the editor using entities and some other rendering
	//TODO: animator and sprite animator have not implemented animation speed of data
	//TODO: make it so that render layers are not specific to a scene meaning the names are created in a spearate manager or elsewhere
	//and then during this constructor they are passed and craeted so each scene has the same layers, but their own instances
	//TODO: make an asset manager that can store custom fonts, images and other stuff so we do not have to make copies or use Globals file
	//TODO: optimize raw text block ,text array and other similar large data structures for holding chars to use less memory
	//TODO: in json serializers there should be a way to match each type to potnetial name sfor properties so we do not have to repeat them multiple times and make mistakes
	//TODO: consolidate the entity searching/retrieval of collection for the global entity manager and that in the scene class
	//TODO: change the scene create entity function to not need to set the newly created transform to that entity 
	// (instead it should be done in entity class by having the class itself set itself for the component)
	//TODO: remove the scene name member from entity and instead try to find a way to group entities with scenes for validateing entities to be in the same scene/entity serialization
	//TODO: main camera should not be set from scenes, but should be based on global camera system/manager
	//TODO: if the level background (or any object) is inside or contains the player at the start, then the gamne crashes usually with a direction not found of colliding body from physics system
	//TODO: since a lot of places need current camera data just for position conversions, maybe conversions should get camera controller as dependency
	//TODO: json serializer should not contain so many duplicate entries (such as font code similar in many places) and should get fonts and other stuff from asset manager
	//TODO: add serialization/deserializetion for particle emitter
	//TODO: if necessary add color gradient GUI and color picker editor GUI
	//TODO: add set minsize possible flag in text gui for settings text gui to min size possible based on font and area given
	//TODO: consider when adding components to entities adding a blank component first and then setting entity so that the constructor with args can use entity pointer/transform data
	//TODO; for entities try to remove the vector of compoennt data and intsead find a way to use the entity mapper to get all components
	//TODO: pressed key state in input manager does not work (key down state lasts for the whole duration without any key press state activation -> maybe remove state?)
	//TODO: make guisettings size enum of MinPossible (gets min size based on gui setting preferred size and area given during render) and MaxPossible that takes up full area given
	//and then integrate into editor system with MinPossible and best sizes for each field and optimizing space use
	//TODO: when pressing the toggle on the editor for gameobject active the program crashes

	static constexpr std::uint8_t TARGET_FPS = 60;

	constexpr std::uint8_t NO_FRAME_LIMIT = -1;
	constexpr std::uint8_t FRAME_LIMIT = NO_FRAME_LIMIT;
	constexpr bool SHOW_FPS = true;
	constexpr bool DO_ENGINE_LOGS = true;

	constexpr std::streamsize DOUBLE_LOG_PRECISION = 8;

	//If true, even if the current output from camera is null, will render default data.
	//This is useful for testing the render loop and finding out FPS (since fps depends on drawing loop)
	constexpr bool ALWAYS_RENDER = true;

	//TODO: maybe default camera render data should just be a preset texture so we dont need to do the same actions every frame
	/*const TextBuffer DEFAULT_RENDER_DATA = TextBuffer(FontData(GLOBAL_FONT_SIZE, GLOBAL_FONT), 
		
		TextBuffer(22, 1, WHITE, {
		{'N', 'O', EMPTY_CHAR_PLACEHOLDER, 'C', 'A', 'M', 'E', 'R', 'A',
		EMPTY_CHAR_PLACEHOLDER, 'O','U', 'T', 'P', 'U', 'T', EMPTY_CHAR_PLACEHOLDER,
		'F', 'O', 'U', 'N', 'D'} });*/

	const KeyboardKey TOGGLE_PAUSE_UPDATE_KEY = KEY_F1;

	constexpr LoopCode SUCCESS_CODE = 0;
	constexpr LoopCode EXIT_CODE = 1;
	constexpr LoopCode ERROR_CODE = 2;

	void Engine::InitEngine()
	{
		InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_NAME.c_str());
		SetTargetFPS(TARGET_FPS);
	}

	void Engine::Destroy()
	{
		CloseWindow();
		EngineLog("DESTROYED ENGINE");
	}

	Engine::Engine() :
		m_assetManager(),
		m_collisionRegistry(),
		m_sceneManager(m_assetManager),
		m_inputManager(m_assetManager),
		m_cameraController(),
		m_physicsManager(m_sceneManager, m_collisionRegistry),
		m_guiSelectorManager(m_inputManager),
		m_transformSystem(),
		m_uiSystem(),
		m_entityRendererSystem(),
		m_lightSystem(m_entityRendererSystem),
		//m_inputSystem(m_inputManager),
		m_spriteAnimatorSystem(m_entityRendererSystem),
		m_animatorSystem(),
		m_collisionBoxSystem(m_collisionRegistry),
		m_physicsBodySystem(m_physicsManager),
		m_playerSystem(m_inputManager),
		m_cameraSystem(&(m_collisionBoxSystem.GetColliderBufferMutable()), &(m_physicsBodySystem.GetLineBufferMutable())),
		m_particleEmitterSystem(),
		m_triggerSystem(),
		//m_playerInfo(std::nullopt),
		//m_mainCameraInfo(std::nullopt),
		m_timeKeeper(),
		m_editor(m_timeKeeper, m_inputManager, m_physicsManager, 
			m_sceneManager, m_cameraController, m_guiSelectorManager, m_collisionBoxSystem)
	{

		EngineLog("FINISHED SYSTEM MANAGERS INIT");

		InitEngine(); 
		EngineLog("FINISHED RAYLIB WINDOW CREATION");

		InitJsonSerializationDependencies(m_sceneManager);

		m_assetManager.InitDependencoes<SceneAsset, GlobalEntityManager, AssetManager>(m_sceneManager.m_GlobalEntityManager, m_assetManager);
		m_assetManager.InitDependencoes<InputProfileAsset, Input::InputManager>(m_inputManager);
		EngineLog("FINISHED ASSET MANAGER DEPENDENCY INIT");
 
		//Note: globals create main menu camera that then adds itself to each scene when scene is loaded
		//TODO: change this weird and akward way of setting camera that feels hidden
		GlobalCreator::CreateGlobals(m_sceneManager.m_GlobalEntityManager, m_sceneManager, m_cameraController);

		//NOTE: we have to load all scenes AFTER all globals are created so that scenes can use globals for deserialization
		//if it is necessary for them (and to prevent misses and potential problems down the line)
		m_sceneManager.LoadAllScenes();
		EngineLog("LOADED ALL SCENES");

		if (!Assert(this, m_sceneManager.TrySetActiveScene(0),
			std::format("Tried to set the active scene to the first one, but failed!")))
			return;

		EngineLog(std::format("SET FIRST SCENE:{}", m_sceneManager.GetActiveScene()->ToString()));
		//m_sceneManager.GetActiveSceneMutable()->InitScene();
		//m_sceneManager.GetActiveSceneMutable()->SetMainCamera(mainCameraEntity);
		LogError(std::format("Scene active: {}", m_sceneManager.GetActiveScene()->ToString()));
		EngineLog("SET FIRST SCENE CAMERA");

		/*
		ECS::Entity& obstacle = m_sceneManager.GetActiveSceneMutable()->CreateEntity("obstacle", TransformData(Vec2{ 20, 20 }));
		
		obstacle.AddComponent<EntityRendererData>(EntityRendererData{
			VisualData({ {TextCharPosition({0,0}, TextChar(GRAY, 'B')) } },
				GetGlobalFont(), VisualData::DEFAULT_FONT_SIZE, VisualData::DEFAULT_CHAR_SPACING, 
				VisualData::DEFAULT_PREDEFINED_CHAR_AREA, VisualData::DEFAULT_PIVOT), RenderLayerType::Player });
				*/

		//Log("CREATING OBSTACLE RB");
		//PhysicsBodyData& obstacleRB= obstacle.AddComponent<PhysicsBodyData>(PhysicsBodyData(0, Vec2(10, 10), Vec2(0, 0)));
		//m_obstacleInfo = ECS::EntityComponentPair<PhysicsBodyData>(obstacle, obstacleRB);

		ECS::Entity& uiIcon= m_sceneManager.GetActiveSceneMutable()->CreateEntity("icon", TransformData(Vec2{ 0, 0 }));
		uiIcon.AddComponent<UIObjectData>(NormalizedPosition(0.1, 0.9));
		uiIcon.AddComponent<EntityRendererData>(EntityRendererData{
			VisualData({ {TextCharPosition({0,0}, TextChar(ORANGE, '*')) } },
				GetGlobalFont(), VisualData::DEFAULT_FONT_SIZE, VisualData::DEFAULT_CHAR_SPACING, 
				VisualData::DEFAULT_PREDEFINED_CHAR_AREA, VisualData::DEFAULT_PIVOT), RenderLayerType::UI });

		/*EngineLog(std::format("LOADED LAYERS FOR SCENE '{}': {}",
			m_sceneManager.GetActiveScene()->GetName(),
			m_sceneManager.GetActiveScene()->ToStringLayers()));*/

		//Log(this, std::format("OBSACLE ID: {}", obstacle.ToString()));
		//Log(this, std::format("CAMERA ID: {}", mainCameraEntity.ToString()));
		//Log(this, std::format("PLAYER ID: {}", playerEntity.ToString()));

		m_inputManager.SetInputCooldown(0.3);
		//InitConsoleCommands();
		m_editor.Init(m_playerSystem);
		EngineLog("ADDED ALL CONSOLE COMMANDS");

		//Assert(false, std::format("FOUND ACTIVE SELECTED: {}", m_guiSelectorManager.TryGetSelectableSelected()->GetLastFrameRect().ToString()));
		/*for (const auto& entity : m_sceneManager.GetActiveSceneMutable()->GetAllEntities())
		{
			LogError(this, std::format("Entity: {} has fields: {}", entity->m_Name,
				Utils::ToStringIterable<std::vector<ComponentField>, ComponentField>(entity->m_Transform.GetFields())));
		}*/
		EngineLog("FINISHED GAME INIT");

		ValidateAll();
	}

	Engine::~Engine()
	{
		//m_commandConsole.DeletePrompts();
	}

	void Engine::ValidateAll()
	{
		m_assetManager.Validate();
		m_sceneManager.ValidateAllScenes();
		m_cameraController.Validate();
		EngineLog("FINISHED VALIDATION");
	}

	void Engine::EngineLog(const std::string& log) const
	{
		if (!DO_ENGINE_LOGS) return;
		Log(this, log, true, false, ANSI_COLOR_BLUE);
	}

	LoopCode Engine::Update()
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("Engine::Update");
#endif 

		m_timeKeeper.UpdateTimeStart();
		const float scaledDeltaTime = m_timeKeeper.GetLastScaledDeltaTime();
		const float unscaledDeltaTime = m_timeKeeper.GetLastIndependentDeltaTime();

		/*if (FRAME_LIMIT != -1 || SHOW_FPS)
		{
			std::cout<<std::format("[ENGINE]: FRAME: {}/{} DELTA_TIME: {} FPS:{} GraphicsFPS:{}\n--------------------------------------------\n",
				std::to_string(m_currentFrameCounter + 1), std::to_string(FRAME_LIMIT), 
				Utils::ToStringDouble(m_deltaTime, DOUBLE_LOG_PRECISION), 
				Utils::ToStringDouble(m_currentFPS, DOUBLE_LOG_PRECISION), std::to_string(GetFPS()));
		}*/

		/*
		if (m_enableDebugInfo)
		{
			m_debugInfo.AddProperty("FPS", std::format("{} fps", std::to_string(GetFPS())));
			m_debugInfo.AddProperty("DeltaTime", std::format("{} s", std::to_string(m_deltaTime)));
			m_debugInfo.AddProperty("TimeStep", std::format("{} s", std::to_string(m_timeStep)));
		}
		*/

		/*m_lastTime = m_currentTime;
		return SUCCESS_CODE;*/

		Scene* activeScene = m_sceneManager.GetActiveSceneMutable();
		if (!Assert(this, activeScene != nullptr, std::format("Tried to update the active scene but there "
			"are none set as active right now", activeScene->GetName())))
			return ERROR_CODE;

		if (!Assert(this, activeScene->HasEntities(), std::format("Tried to update the active scene:{} but there "
			"are no entities in the scene", activeScene->GetName())))
			return ERROR_CODE;

		m_cameraController.UpdateActiveCamera();
		CameraData& mainCamera = m_cameraController.GetActiveCameraMutable();

		std::string cameraSceneName = mainCamera.GetEntitySafe().GetSceneName();
		if (!Assert(this, cameraSceneName== ECS::Entity::GLOBAL_SCENE_NAME || cameraSceneName == activeScene->GetName(), 
			std::format("Tried to get active camera:{} during update loop, "
			"but that camera is not in the active scene OR global storage (main camera scene:{}, active scene:{})", mainCamera.ToString(),
			cameraSceneName, activeScene->GetName())))
			return ERROR_CODE;

		/*if (!Assert(this, activeScene->HasMainCamera(), std::format("Tried to update the active scene: {}, "
			"but it has no main camera", activeScene->GetName())))
			return ERROR_CODE;
		
		CameraData* mainCamera = activeScene->TryGetMainCameraMutable();
		ECS::Entity* mainCameraEntity = activeScene->TryGetMainCameraEntityMutable();

		if (!Assert(this, mainCamera != nullptr && mainCameraEntity != nullptr,
			std::format("Tried to update the active scene:{} but failed to retrieve "
				"main camera(found:{}) and/or its entity(found:{})", activeScene->GetName(),
				std::to_string(mainCamera != nullptr), std::to_string(mainCameraEntity != nullptr))))
			return ERROR_CODE;*/


		/*if (!Assert(this, m_playerInfo.has_value(),
			std::format("Tried to update the active scene:{} but failed to get "
				"player info in a valid state", activeScene->GetName())))
			return ERROR_CODE;*/

		//We need to reset to default since previous changes were baked into the buffer
		//so we need to clear it for a fresh update

		//for (auto& layer : activeScene->GetLayersMutable())
		//{
		//	//Log(std::format("Resetting to  default layer: {}/{}", std::to_string(i), std::to_string(m_Layers.size()-1)));
		//	layer->ResetToDefault();
		//}
	
		//TODO: maybe some general scene stuff should be abstracted into scene manager
		activeScene->ResetAllLayers();
		activeScene->ResetFrameDirtyComponentCount();

		//TODO: ideally the systems would be supplied with only relevenat components without the need of entities
		//but this can only be the case if data is stored directyl without std::any and linear component data for same entities is used

		m_transformSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
		m_uiSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);

		m_inputManager.Update(unscaledDeltaTime);
		//if (m_enableDebugInfo)
		//{
		//	m_debugInfo.AddProperty("KeysDown", Utils::ToStringIterable<std::vector<std::string>, 
		//		std::string>(m_inputManager.GetAllKeysWithStateAsString(Input::KeyState::Down)));
		//}

		//m_inputSystem.SystemUpdate(*activeScene, m_playerInfo.value().GetAt<2>(), *(m_playerInfo.value().m_Entity), m_deltaTime);

		m_playerSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
		//if (m_enableDebugInfo) m_debugInfo.AddProperty("Input", std::format("{}", m_playerInfo.value().GetAt<0>().GetFrameInput().ToString()));

		m_collisionBoxSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
		m_physicsManager.GetPhysicsWorldMutable().UpdateStart(scaledDeltaTime);
		/*if (m_enableDebugInfo)
		{
			m_debugInfo.AddProperty("PlayerPos", std::format("{} m", m_playerInfo.value().m_Entity->m_Transform.m_Pos.ToString()));
			m_debugInfo.AddProperty("PlayerVel", std::format("{} m/s", m_playerInfo.value().GetAt<1>().GetVelocity().ToString(3, VectorForm::Component)));
			m_debugInfo.AddProperty("PlayerAcc", std::format("{} m/s2", m_playerInfo.value().GetAt<1>().GetAcceleration().ToString(3, VectorForm::Component)));
			m_debugInfo.AddProperty("Grounded:", std::format("{}", std::to_string(m_playerInfo.value().GetAt<0>().GetIsGrounded())));
			m_debugInfo.AddProperty("GroundDist:", std::format("{} m", std::to_string(m_playerInfo.value().GetAt<0>().GetVerticalDistanceToGround())));
		}*/

		m_physicsBodySystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
		m_triggerSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
	/*	Log(std::format("Player POS: {} SCREEN POS: {}", m_playerInfo.value().m_Entity->m_Transform.m_Pos.ToString(), 
			Conversions::WorldToScreenPosition(*mainCamera, m_playerInfo.value().m_Entity->m_Transform.m_Pos).ToString()));*/
		
		m_animatorSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
		m_spriteAnimatorSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
		m_particleEmitterSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);

		/*LogError(this, std::format("Player visual: {} scene entities: {}", m_playerInfo.value().m_Entity->TryGetComponent<EntityRendererData>()->GetVisualData().ToString(), 
			std::to_string(activeScene->GetEntityCount())));*/
		//LogError(this, std::format("Scene entities: {}", activeScene->ToString()));
		//LogError(activeScene->GetAllEntities()[0]->GetName());
		
		//TODO: it seems enttiy system is causing a stirng to long exception
		m_entityRendererSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
		
		//LogWarning(this, std::format("PLAYER OBSTACLE COLLISION: {} PLAYER POS: {} (PLAYER RECT: {}) last input: {} velocity: {} OBSTACLE POS: {} (OBstacle REDCT: {}) ", 
		//	std::to_string(Physics::DoBodiesIntersect(m_playerInfo.value().GetAt<1>(), *(m_obstacleInfo.value().m_Data))),
		//	m_playerInfo.value().m_Entity->m_Transform.m_Pos.ToString(),
		//	//TODO: this is technically wrong since we use transform pos as center for aabb global pos but it could be offset (just for testing when offset=0)
		//	m_playerInfo.value().GetAt<1>().GetAABB().ToString(m_playerInfo.value().m_Entity->m_Transform.m_Pos),
		//	m_playerInfo.value().GetAt<0>().GetFrameInput().ToString(),
		//	m_playerInfo.value().GetAt<1>().GetVelocity().ToString(),
		//	m_obstacleInfo.value().m_Entity->m_Transform.m_Pos.ToString(),
		//	m_obstacleInfo.value().m_Data->GetAABB().ToString(m_obstacleInfo.value().m_Entity->m_Transform.m_Pos)));
		//TODO: light system without any other problems drop frames to ~20 fps
		m_lightSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);

		m_cameraSystem.SystemUpdate(*activeScene, mainCamera, mainCamera.GetEntitySafeMutable(), scaledDeltaTime);
		const TextBufferMixed& collapsedBuffer = m_cameraSystem.GetCurrentFrameBuffer();
		Assert(this, !collapsedBuffer.empty(), std::format("Tried to render buffer from camera output, but it has no data"));

		/*if (m_enableCommandConsole) m_commandConsole.Update();*/

		/*if (m_enableDebugInfo)
		{
			Vector2 mousePos = GetMousePosition();
			ScreenPosition mouseScreenPos = { static_cast<int>(mousePos.x), static_cast<int>(mousePos.y)};
			WorldPosition mouseWorld = Conversions::ScreenToWorldPosition(*m_mainCameraInfo.value().m_Data, mouseScreenPos);
			m_debugInfo.SetMouseDebugData(DebugMousePosition{ mouseWorld, {mouseScreenPos.m_X+15, mouseScreenPos.m_Y} });
		}*/

		/*m_entityEditor.Update(mainCamera);*/
		m_guiSelectorManager.Update();
		m_editor.Update(unscaledDeltaTime, m_timeKeeper.GetTimeScale(), *activeScene, mainCamera);
		//Assert(false, std::format("Found selectables:{}", m_guiSelectorManager.T));

		//TODO: rendering buffer drops frames
		if (!collapsedBuffer.empty())
		{
			Rendering::RenderBuffer(collapsedBuffer, m_cameraSystem.GetCurrentColliderOutlineBuffer(),
				m_cameraSystem.GetCurrentLineBuffer(), std::vector<IBasicRenderable*>{&m_editor});
				//m_enableCommandConsole? &m_commandConsole : nullptr, &m_entityEditor);
		}
		//else if (ALWAYS_RENDER) Rendering::RenderBuffer(DEFAULT_RENDER_DATA, RENDER_INFO);

		m_transformSystem.UpdateLastFramePos(*activeScene);
		//if (m_enableDebugInfo) m_debugInfo.ClearProperties();

		//Log(std::format("Player pos: {}", m_playerInfo.value().m_Entity->m_Transform.m_Pos.ToString()));
		//Log(m_sceneManager.GetActiveScene()->ToStringLayers());

		m_timeKeeper.UpdateTimeEnd();

		if (m_timeKeeper.ReachedFrameLimit()) 
			return EXIT_CODE;

		return SUCCESS_CODE;
	}

	void Engine::BeginUpdateLoop()
	{
		LoopCode currentCode = SUCCESS_CODE;
		while (!WindowShouldClose())
		{
			if (IsKeyPressed(TOGGLE_PAUSE_UPDATE_KEY))
			{
				std::cin.get();
				Utils::ClearSTDCIN();
			}

			try
			{
				currentCode = Update();
			}
			catch (const std::exception& e)
			{
				LogError(this, std::format("Update loop terminated due to exception: {}", e.what()), true, false, true);
				return;
			}

#ifdef ENABLE_PROFILER
			ProfilerTimer::m_Profiler.LogCurrentRoundTimes();
#endif
			if (!Assert(this, currentCode != ERROR_CODE, 
				std::format("Update loop terminated due to error"), true)) 
				return;

			if (!Assert(this, currentCode != EXIT_CODE,
				std::format("Update loop terminated due to loop end triggered"), true)) 
				return;
		}
	}
}
