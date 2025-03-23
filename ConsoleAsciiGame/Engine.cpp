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
#include "InputSystem.hpp"
#include "AnimatorSystem.hpp"
#include "SpriteAnimatorSystem.hpp"
#include "PhysicsBodySystem.hpp"
#include "Array2DPosition.hpp"
#include "Timer.hpp"
#include "ProfilerTimer.hpp"
#include "PositionConversions.hpp"
#include "DebugInfo.hpp"

namespace Core
{
	//-------------------------------------------------------------------
	// GLOBAL TODO CHECKLIST
	//-------------------------------------------------------------------
	//TODO: maybe make an array version for text buffer (actually this time)
	//TODO: for performance make all members, function params that are const std::string that do not need to be vars (like are just direct times) as const char* (const char[])
	//TODO: add vector class instead of point (can just rename
	//TODO: perhaps vec2 (my own vector) should be merged with point2d since vec2 point can be interpreted from a vec2 data
	//TODO: currentyl the camera maps world pos to its pos based on whole screen to accureately position it basewd on the viewport, but it does not manipulate the size 
	//meaning that for example a pos at the top left will maintain its distance from the top edge in the camera output but say something like size of a collider
	//will keep its size from world pos to screen pos (making it inconcistent with sizing)
	//TODO: input component is basically useless now should it be removed?
	//TODO: maybe combine all key,gamepad buttons into one enum and then separate check them based on its enum internal value within input manager
	//to not need dependence on raylib for keyboardkey and other input stuff and to keep it an implementation detail
	//TODO: change command prompt to use the new inputfield type
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
	//TODO: rather than each scene having its own physics world pheraps there should only be one main one that is stored here and it can manage adding and removing entities
	//TODO: remove the scene name member from entity and instead try to find a way to group entities with scenes for validateing entities to be in the same scene/entity serialization
	//TODO: main camera should not be set from scenes, but should be based on global camera system/manager

	static const std::string SCENES_PATH = "scenes";
	static const std::string INPUT_PROFILES_PATH = "input";
	static constexpr std::uint8_t TARGET_FPS = 60;

	constexpr std::uint8_t NO_FRAME_LIMIT = -1;
	constexpr std::uint8_t FRAME_LIMIT = NO_FRAME_LIMIT;
	constexpr bool SHOW_FPS = true;

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
	const KeyboardKey TOGGLE_DEBUG_INFO_KEY = KEY_TAB;
	const KeyboardKey TOGGLE_COMMAND_CONSOLE_KEY = KEY_TAB;

	constexpr LoopCode SUCCESS_CODE = 0;
	constexpr LoopCode END_CODE = 1;
	constexpr LoopCode ERROR_CODE = 2;

	void Engine::Init()
	{
		InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_NAME.c_str());
		SetTargetFPS(TARGET_FPS);

		std::cout << "\n\n[ENGINE]: INITIALIZED RAYLIB WINDOW" << std::endl;
	}

	void Engine::Destroy()
	{
		CloseWindow();
	}

	Engine::Engine() :
		m_sceneManager(SCENES_PATH),
		m_inputManager(INPUT_PROFILES_PATH),
		m_physicsManager(m_sceneManager),
		m_guiSelectorManager(m_inputManager),
		m_transformSystem(),
		m_uiSystem(),
		m_entityRendererSystem(),
		m_lightSystem(m_entityRendererSystem),
		//m_inputSystem(m_inputManager),
		m_spriteAnimatorSystem(m_entityRendererSystem),
		m_animatorSystem(*this),
		m_physicsBodySystem(m_physicsManager),
		m_playerSystem(m_inputManager),
		m_cameraSystem(&(m_physicsBodySystem.GetColliderBufferMutable()), &(m_physicsBodySystem.GetLineBufferMutable())),
		m_currentTime(std::chrono::high_resolution_clock().now()),
		m_lastTime(std::chrono::high_resolution_clock().now()),
		m_playerInfo(std::nullopt),
		m_mainCameraInfo(std::nullopt),
		m_debugInfo{}, 
		m_enableDebugInfo(false),
		m_commandConsole(m_inputManager, m_guiSelectorManager),
		m_enableCommandConsole(false),
		m_entityEditor(m_inputManager, m_sceneManager, m_physicsManager, m_guiSelectorManager)
	{
		Init(); 
		InitJsonSerializers(m_sceneManager);

		VisualDataPreset visualPreset = { GetGlobalFont(), VisualData::DEFAULT_FONT_SIZE, VisualData::DEFAULT_CHAR_SPACING,
				CharAreaType::Predefined, VisualData::DEFAULT_PREDEFINED_CHAR_AREA, VisualData::DEFAULT_PIVOT };

		ECS::Entity& playerEntity = m_sceneManager.m_GlobalEntityManager.CreateGlobalEntity("player", TransformData(Vec2{ 10, 5 }));
		PhysicsBodyData& playerRB = playerEntity.AddComponent<PhysicsBodyData>(PhysicsBodyData(1, Vec2(2, 2), Vec2(0, 0), GRAVITY, 20));

		PlayerData& playerData = playerEntity.AddComponent<PlayerData>(PlayerData(playerRB, 5, 20));

		//InputData& inputData = playerEntity.AddComponent<InputData>(InputData{});
		LightSourceData& lightSource= playerEntity.AddComponent<LightSourceData>(LightSourceData{ 8, RenderLayerType::Background,
			ColorGradient(Color(243, 208, 67, 255), Color(228, 8, 10, 255)), std::uint8_t(254), 1.2f });

		//Assert(false, std::format("player light source:{}", playerEntity.TryGetComponentWithName("LightSourceData")->ToString()));
		
		//Log("CREATING PLAYER RB");
		//ComponentData* ptr = &lightSource;
		//Assert(false, std::format("LIGHT SOURCE PTR:{}", typeid(*ptr).name()));
		//Assert(false, std::format("Light source: {}", typeid(*ptr).name()));
		//Assert(false, std::format("Light source fields: {}", lightSource.ToStringFields()));

		playerEntity.AddComponent<EntityRendererData>(EntityRendererData{
			VisualData({ {TextCharPosition({0,0}, TextChar(GRAY, 'H')) } },visualPreset), RenderLayerType::Player});

		ComponentFieldReference lightRadiusref = ComponentFieldReference(&lightSource, "Radius");
		//Assert(false, std::format("Entity light radius: {}", lightRadiusref.m_Entity->ToString()));
		ComponentField& field= lightRadiusref.GetComponentFieldSafeMutable();
		//Assert(false, std::format("Light source ref field: {}", field.ToString()));

		playerEntity.AddComponent<AnimatorData>(AnimatorData(std::vector<AnimationPropertyVariant>{
				AnimationProperty<std::uint8_t>(lightRadiusref, {
				AnimationPropertyKeyframe<std::uint8_t>(std::uint8_t(8), 0),
				AnimationPropertyKeyframe<std::uint8_t>(std::uint8_t(1), 1)})}, 1, 1, true));

		playerEntity.AddComponent<SpriteAnimatorData>(SpriteAnimatorData(
			{ SpriteAnimationFrame(0, VisualData(RawTextBufferBlock{{TextCharPosition({}, TextChar(WHITE, 'O'))}}, visualPreset)),
			  SpriteAnimationFrame(2, VisualData(RawTextBufferBlock{{TextCharPosition({}, TextChar(WHITE, '4'))}}, visualPreset)) }, 1, 4, true));

		m_playerInfo = ECS::EntityComponents<PlayerData, PhysicsBodyData>{ playerEntity, playerData, playerRB };

		ECS::Entity& mainCameraEntity = m_sceneManager.m_GlobalEntityManager.CreateGlobalEntity("MainCamera", TransformData(Vec2{ 0, 0 }));
		CameraData& cameraData = mainCameraEntity.AddComponent<CameraData>(CameraData{ CameraSettings{SCREEN_ASPECT_RATIO, 120, &playerEntity} });
		m_mainCameraInfo = ECS::EntityComponentPair<CameraData>{ mainCameraEntity, cameraData };

		//NOTE: we have to load all scenes AFTER all globals are created so that scenes can use globals for deserialization
		//if it is necessary for them (and to prevent misses and potential problems down the line)
		m_sceneManager.LoadAllScenes();

		if (!Assert(this, m_sceneManager.TrySetActiveScene(0),
			std::format("Tried to set the active scene to the first one, but failed!")))
			return;

		//m_sceneManager.GetActiveSceneMutable()->InitScene();
		m_sceneManager.GetActiveSceneMutable()->SetMainCamera(mainCameraEntity);
		LogError(std::format("Scene active: {}", m_sceneManager.GetActiveScene()->ToString()));


		ECS::Entity& obstacle = m_sceneManager.GetActiveSceneMutable()->CreateEntity("obstacle", TransformData(Vec2{ 20, 20 }));
		//Log(std::format("Has entity with id: {}", m_sceneManager.GetActiveSceneMutable()->TryGetEntityMutable(obstacle.m_Id)->ToString()));
		
		obstacle.AddComponent<EntityRendererData>(EntityRendererData{
			VisualData({ {TextCharPosition({0,0}, TextChar(GRAY, 'B')) } },
				GetGlobalFont(), VisualData::DEFAULT_FONT_SIZE, VisualData::DEFAULT_CHAR_SPACING, 
				VisualData::DEFAULT_PREDEFINED_CHAR_AREA, VisualData::DEFAULT_PIVOT), RenderLayerType::Player });

		Log("CREATING OBSTACLE RB");
		PhysicsBodyData& obstacleRB= obstacle.AddComponent<PhysicsBodyData>(PhysicsBodyData(0, Vec2(10, 10), Vec2(0, 0)));
		m_obstacleInfo = ECS::EntityComponentPair<PhysicsBodyData>(obstacle, obstacleRB);

		ECS::Entity& uiIcon= m_sceneManager.GetActiveSceneMutable()->CreateEntity("icon", TransformData(Vec2{ 0, 0 }));
		uiIcon.AddComponent<UIObjectData>(NormalizedPosition(0.1, 0.9));
		uiIcon.AddComponent<EntityRendererData>(EntityRendererData{
			VisualData({ {TextCharPosition({0,0}, TextChar(ORANGE, '*')) } },
				GetGlobalFont(), VisualData::DEFAULT_FONT_SIZE, VisualData::DEFAULT_CHAR_SPACING, 
				VisualData::DEFAULT_PREDEFINED_CHAR_AREA, VisualData::DEFAULT_PIVOT), RenderLayerType::UI });

		std::cout << "\n\n[ENGINE]: INITIALIZED GLOBALS" << std::endl;
		std::cout << std::format("[ENGINE]: LOADED LAYERS FOR SCENE '{}': {}", 
			m_sceneManager.GetActiveScene()->GetName(),
			m_sceneManager.GetActiveScene()->ToStringLayers()) << std::endl;

		Log(this, std::format("OBSACLE ID: {}", obstacle.ToString()));
		Log(this, std::format("CAMERA ID: {}", mainCameraEntity.ToString()));
		Log(this, std::format("PLAYER ID: {}", playerEntity.ToString()));

		m_inputManager.SetInputCooldown(0.3);
		InitConsoleCommands();

		//Assert(false, std::format("FOUND ACTIVE SELECTED: {}", m_guiSelectorManager.TryGetSelectableSelected()->GetLastFrameRect().ToString()));
		/*for (const auto& entity : m_sceneManager.GetActiveSceneMutable()->GetAllEntities())
		{
			LogError(this, std::format("Entity: {} has fields: {}", entity->m_Name,
				Utils::ToStringIterable<std::vector<ComponentField>, ComponentField>(entity->m_Transform.GetFields())));
		}*/

		ValidateAll();
	}

	Engine::~Engine()
	{
		m_commandConsole.DeletePrompts();
	}

	void Engine::ValidateAll()
	{
		m_sceneManager.ValidateAllScenes();
	}

	void Engine::InitConsoleCommands()
	{
		m_commandConsole.AddPrompt(new CommandPrompt<std::string, float, float>("setpos", {"EntityName", "PosX", "PosY"},
			[this](const std::string& entityName, const float& x, const float& y) -> void {
				if (ECS::Entity* entity= m_sceneManager.GetActiveSceneMutable()->TryGetEntityMutable(entityName, true))
				{
					entity->m_Transform.SetPos({ x, y });
				}
			}));

		m_commandConsole.AddPrompt(new CommandPrompt<std::string>("editor", { "EntityName"},
			[this](const std::string& entityName) -> void {
				if (ECS::Entity* entity = m_sceneManager.GetActiveSceneMutable()->TryGetEntityMutable(entityName, true))
				{
					//Assert(false, std::format("Sending entity: {}", entity->m_Name));
					m_entityEditor.SetEntityGUI(*entity);
					return;
				}
				m_commandConsole.LogOutputMessage(std::format("Entity with name: '{}' could not be found", 
					entityName), ConsoleOutputMessageType::Error);
			}));

		m_commandConsole.AddPrompt(new CommandPrompt<>("docs", std::vector<std::string>{},
			[this]() -> void {
				m_commandConsole.LogOutputMessagesUnrestricted(
					m_commandConsole.GetPromptDocumentationAll(), ConsoleOutputMessageType::Default);
			}));

		m_commandConsole.AddPrompt(new CommandPrompt<std::string>("debugmessage", std::vector<std::string>{"MessageFilter"},
			[](const std::string& messageFilter) -> void {
				SetLogMessageFilter(messageFilter);
			}));

		m_commandConsole.AddPrompt(new CommandPrompt<std::string>("debugtype", std::vector<std::string>{"MessageType"},
			[this](const std::string& typeFilter) -> void {
				std::optional<LogType> maybeLogType = StringToLogType(typeFilter);
				if (maybeLogType == std::nullopt)
				{
					m_commandConsole.LogOutputMessage(std::format("Invalid LogType: {}", 
						typeFilter), ConsoleOutputMessageType::Error);
					return;
				}

				SetLogTypeFilter(maybeLogType.value());
				/*LogError(std::format("The new log type: {} has log: {}", LogTypeToString(GetLogTypeFilter()), 
					std::to_string(Utils::HasFlagAny(GetLogTypeFilter(), LogType::Log))));*/
				//throw std::invalid_argument("POOP");
			}));

		m_commandConsole.AddPrompt(new CommandPrompt<>("debugreset", std::vector<std::string>{},
			[this]() -> void {
				ResetLogFilters();
			}));

		m_commandConsole.AddPrompt(new CommandPrompt<float>("settimestep", std::vector<std::string>{"TimeStep"},
			[this](const float& timeStep) -> void {
				m_timeStep = timeStep;
			}));

		m_commandConsole.AddPrompt(new CommandPrompt<int>("debugmark", std::vector<std::string>{"Index"},
			[this](const int& index) -> void {
				if (index == -1)
				{
					m_debugInfo.ClearHighlightedIndices();
					return;
				}

				if (index < 0 || index >= m_debugInfo.GetText().size())
				{
					m_commandConsole.LogOutputMessage(std::format("Invalid Index: {}", 
						std::to_string(index)), ConsoleOutputMessageType::Error);
					return;
				}

				if (!m_debugInfo.TryAddHighlightedIndex(static_cast<std::size_t>(index)))
				{
					m_commandConsole.LogOutputMessage(std::format("Index already added: {}",
						std::to_string(index)), ConsoleOutputMessageType::Error);
					return;
				}
			}));

		m_commandConsole.AddPrompt(new CommandPrompt<bool>("cheats", std::vector<std::string>{"CheapStatus"},
			[this](const bool& enableCheats) -> void {
				m_playerSystem.SetCheatStatus(enableCheats);
			}));
	}

	LoopCode Engine::Update()
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("Engine::Update");
#endif 

		m_currentTime = std::chrono::high_resolution_clock().now();
		m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(m_currentTime - m_lastTime).count() / static_cast<float>(1000) * m_timeStep;
		m_currentFPS = 1 / m_deltaTime;

		/*if (FRAME_LIMIT != -1 || SHOW_FPS)
		{
			std::cout<<std::format("[ENGINE]: FRAME: {}/{} DELTA_TIME: {} FPS:{} GraphicsFPS:{}\n--------------------------------------------\n",
				std::to_string(m_currentFrameCounter + 1), std::to_string(FRAME_LIMIT), 
				Utils::ToStringDouble(m_deltaTime, DOUBLE_LOG_PRECISION), 
				Utils::ToStringDouble(m_currentFPS, DOUBLE_LOG_PRECISION), std::to_string(GetFPS()));
		}*/
		if (m_enableDebugInfo)
		{
			m_debugInfo.AddProperty("FPS", std::format("{} fps", std::to_string(GetFPS())));
			m_debugInfo.AddProperty("DeltaTime", std::format("{} s", std::to_string(m_deltaTime)));
			m_debugInfo.AddProperty("TimeStep", std::format("{} s", std::to_string(m_timeStep)));
		}

		/*m_lastTime = m_currentTime;
		return SUCCESS_CODE;*/

		Scene* activeScene = m_sceneManager.GetActiveSceneMutable();
		if (!Assert(this, activeScene != nullptr, std::format("Tried to update the active scene but there "
			"are none set as active right now", activeScene->GetName())))
			return ERROR_CODE;

		if (!Assert(this, activeScene->HasMainCamera(), std::format("Tried to update the active scene: {}, "
			"but it has no main camera", activeScene->GetName())))
			return ERROR_CODE;

		if (!Assert(this, activeScene->HasEntities(), std::format("Tried to update the active scene:{} but there "
			"are no entities in the scene", activeScene->GetName())))
			return ERROR_CODE;
		
		CameraData* mainCamera = activeScene->TryGetMainCameraMutable();
		ECS::Entity* mainCameraEntity = activeScene->TryGetMainCameraEntityMutable();

		if (!Assert(this, mainCamera != nullptr && mainCameraEntity != nullptr,
			std::format("Tried to update the active scene:{} but failed to retrieve "
				"main camera(found:{}) and/or its entity(found:{})", activeScene->GetName(),
				std::to_string(mainCamera != nullptr), std::to_string(mainCameraEntity != nullptr))))
			return ERROR_CODE;

		if (!Assert(this, m_playerInfo.has_value(),
			std::format("Tried to update the active scene:{} but failed to get "
				"player info in a valid state", activeScene->GetName())))
			return ERROR_CODE;

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

		m_transformSystem.SystemUpdate(*activeScene, m_deltaTime);
		m_uiSystem.SystemUpdate(*activeScene, m_deltaTime);

		m_inputManager.Update(m_deltaTime);
		if (m_enableDebugInfo)
		{
			m_debugInfo.AddProperty("KeysDown", Utils::ToStringIterable<std::vector<std::string>, 
				std::string>(m_inputManager.GetAllKeysWithStateAsString(Input::KeyState::Down)));
		}

		//m_inputSystem.SystemUpdate(*activeScene, m_playerInfo.value().GetAt<2>(), *(m_playerInfo.value().m_Entity), m_deltaTime);

		m_playerSystem.SystemUpdate(*activeScene, m_playerInfo.value().GetAt<0>(), *(m_playerInfo.value().m_Entity), m_deltaTime);
		if (m_enableDebugInfo) m_debugInfo.AddProperty("Input", std::format("{}", m_playerInfo.value().GetAt<0>().GetFrameInput().ToString()));

		m_physicsManager.GetPhysicsWorldMutable().UpdateStart(m_deltaTime);
		if (m_enableDebugInfo)
		{
			m_debugInfo.AddProperty("PlayerPos", std::format("{} m", m_playerInfo.value().m_Entity->m_Transform.m_Pos.ToString()));
			m_debugInfo.AddProperty("PlayerVel", std::format("{} m/s", m_playerInfo.value().GetAt<1>().GetVelocity().ToString(3, VectorForm::Component)));
			m_debugInfo.AddProperty("PlayerAcc", std::format("{} m/s2", m_playerInfo.value().GetAt<1>().GetAcceleration().ToString(3, VectorForm::Component)));
			m_debugInfo.AddProperty("Grounded:", std::format("{}", std::to_string(m_playerInfo.value().GetAt<0>().GetIsGrounded())));
			m_debugInfo.AddProperty("GroundDist:", std::format("{} m", std::to_string(m_playerInfo.value().GetAt<0>().GetVerticalDistanceToGround())));
		}

		m_physicsBodySystem.SystemUpdate(*activeScene, m_deltaTime);
		Log(std::format("Player POS: {} SCREEN POS: {}", m_playerInfo.value().m_Entity->m_Transform.m_Pos.ToString(), 
			Conversions::WorldToScreenPosition(*mainCamera, m_playerInfo.value().m_Entity->m_Transform.m_Pos).ToString()));
		
		m_animatorSystem.SystemUpdate(*activeScene, m_deltaTime);
		m_spriteAnimatorSystem.SystemUpdate(*activeScene, m_deltaTime);
		

		LogError(this, std::format("Player visual: {} scene entities: {}", m_playerInfo.value().m_Entity->TryGetComponent<EntityRendererData>()->GetVisualData().ToString(), 
			std::to_string(activeScene->GetEntityCount())));
		LogError(this, std::format("Scene entities: {}", activeScene->ToString()));
		//LogError(activeScene->GetAllEntities()[0]->GetName());
		
		//TODO: it seems enttiy system is causing a stirng to long exception
		m_entityRendererSystem.SystemUpdate(*activeScene, m_deltaTime);
		
		LogWarning(this, std::format("PLAYER OBSTACLE COLLISION: {} PLAYER POS: {} (PLAYER RECT: {}) last input: {} velocity: {} OBSTACLE POS: {} (OBstacle REDCT: {}) ", 
			std::to_string(Physics::DoBodiesIntersect(m_playerInfo.value().GetAt<1>(), *(m_obstacleInfo.value().m_Data))),
			m_playerInfo.value().m_Entity->m_Transform.m_Pos.ToString(),
			//TODO: this is technically wrong since we use transform pos as center for aabb global pos but it could be offset (just for testing when offset=0)
			m_playerInfo.value().GetAt<1>().GetAABB().ToString(m_playerInfo.value().m_Entity->m_Transform.m_Pos),
			m_playerInfo.value().GetAt<0>().GetFrameInput().ToString(),
			m_playerInfo.value().GetAt<1>().GetVelocity().ToString(),
			m_obstacleInfo.value().m_Entity->m_Transform.m_Pos.ToString(),
			m_obstacleInfo.value().m_Data->GetAABB().ToString(m_obstacleInfo.value().m_Entity->m_Transform.m_Pos)));
		//TODO: light system without any other problems drop frames to ~20 fps
		m_lightSystem.SystemUpdate(*activeScene, m_deltaTime);

		m_cameraSystem.SystemUpdate(*activeScene, *mainCamera, *mainCameraEntity, m_deltaTime);
		const TextBufferMixed& collapsedBuffer = m_cameraSystem.GetCurrentFrameBuffer();
		Assert(this, !collapsedBuffer.empty(), std::format("Tried to render buffer from camera output, but it has no data"));

		if (m_enableCommandConsole) m_commandConsole.Update();

		if (m_enableDebugInfo)
		{
			Vector2 mousePos = GetMousePosition();
			ScreenPosition mouseScreenPos = { static_cast<int>(mousePos.x), static_cast<int>(mousePos.y)};
			WorldPosition mouseWorld = Conversions::ScreenToWorldPosition(*m_mainCameraInfo.value().m_Data, mouseScreenPos);
			m_debugInfo.SetMouseDebugData(DebugMousePosition{ mouseWorld, {mouseScreenPos.m_X+15, mouseScreenPos.m_Y} });
		}

		m_entityEditor.Update();
		m_guiSelectorManager.Update();

		//TODO: rendering buffer drops frames
		if (!collapsedBuffer.empty())
		{
			Rendering::RenderBuffer(collapsedBuffer, m_cameraSystem.GetCurrentColliderOutlineBuffer(), 
				m_cameraSystem.GetCurrentLineBuffer(), m_enableDebugInfo? &m_debugInfo : nullptr, 
				m_enableCommandConsole? &m_commandConsole : nullptr, &m_entityEditor);
		}
		//else if (ALWAYS_RENDER) Rendering::RenderBuffer(DEFAULT_RENDER_DATA, RENDER_INFO);

		m_transformSystem.UpdateLastFramePos(*activeScene);
		m_physicsManager.GetPhysicsWorldMutable().UpdateEnd();
		if (m_enableDebugInfo) m_debugInfo.ClearProperties();

		//Log(std::format("Player pos: {}", m_playerInfo.value().m_Entity->m_Transform.m_Pos.ToString()));
		//Log(m_sceneManager.GetActiveScene()->ToStringLayers());

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
			if (IsKeyPressed(TOGGLE_PAUSE_UPDATE_KEY))
			{
				std::cin.get();
				Utils::ClearSTDCIN();
			}
			if (IsKeyPressed(TOGGLE_DEBUG_INFO_KEY))
			{
				m_enableDebugInfo = !m_enableDebugInfo;
			}
			if (IsKeyPressed(TOGGLE_COMMAND_CONSOLE_KEY))
			{
				m_enableCommandConsole = !m_enableCommandConsole;
				if (!m_enableCommandConsole) m_commandConsole.ResetInput();
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

			//We do not terminate update if we want to play a few frames
			if (FRAME_LIMIT != NO_FRAME_LIMIT && m_currentFrameCounter < FRAME_LIMIT) continue;

			if (!Assert(this, currentCode != ERROR_CODE, 
				std::format("Update loop terminated due to error"), true)) return;

			if (!Assert(this, currentCode != END_CODE,
				std::format("Update loop terminated due to loop end triggered"), true)) return;
		}
	}
}
