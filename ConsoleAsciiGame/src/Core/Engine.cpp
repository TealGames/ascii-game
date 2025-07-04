#include "pch.hpp"
#include "Core/Engine.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "raylib.h"
#include "StaticGlobals.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Core/Rendering/GameRenderer.hpp"
#include "Utils/HelperFunctions.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"
#include "ECS/Systems/Types/World/EntityRendererSystem.hpp"
#include "ECS/Systems/Types/World/CameraSystem.hpp"
#include "ECS/Systems/Types/World/LightSourceSystem.hpp"
#include "ECS/Systems/Types/World/InputSystem.hpp"
#include "ECS/Systems/Types/World/AnimatorSystem.hpp"
#include "ECS/Systems/Types/World/SpriteAnimatorSystem.hpp"
#include "ECS/Systems/Types/World/PhysicsBodySystem.hpp"
#include "Core/UIElementTemplates.hpp"
#include "Utils/Data/Array2DPosition.hpp"
#include "Core/Time/Timer.hpp"
#include "Core/Analyzation/ProfilerTimer.hpp"
#include "Core/PositionConversions.hpp"
#include "Core/Analyzation/DebugInfo.hpp"
#include "Core/Input/InputProfileAsset.hpp"
#include "Core/Serialization/JsonSerializers.hpp"
#include "Game/GlobalCreator.hpp"
#include "Core/Asset/GlobalColorCodes.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Fig/Fig.hpp"
#include "AnsiCodes.hpp"

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
	//TODO: update the visual data system so that instead of having to place characcters in grid we can place them in any position (vec2) from the pivot pos
	//by having the visual data store it based off of grid (like maybe a preset setting that has default char spacing for each char) or custom ones for each character
	//TODO: the asset system is a little akward with dependencies. Scene asset should ideally noy depeend on the asset manager but should get its level asset via a depedency on another asset
	// or file from constructor. LAso scene asset should not have to be laoded separately by another class that is not the asset manager. so maybe there should be a setup function 
	//in the asset amanger to do so before it is added
	//TODO: it is weird that the json serializes have the asset and scene manager dependecies which will eventually get added and linked to the files that all use serializers
	//and it should instead be a dependecy injection of those types into the deserialization functions (maybe event split deserializetion/serialization of components into separate class?)
	//and remove the depedneyc on json and the implicit need for scene manager/asset manager hidden via the serializers
	//TODO: make some of the colors for the editor and sizing global values, as well as ways to get global text gui settings for the buttons, text
	//TODO: extract some of the editor and gameplay engine code so that the engine does not include any dependencies to editor
	//TODO: there is gui rect and render info both of which have nearly identical data so pick one to use consistently
	//TODO: checkbox on component field gui throws error when selected
	//TODO: the reason why lot of editor/event/callback stuff is getting invalidated/thrown errors because there may be some moves (like using push instead of emplace)
	//which then cause any function with "this" capture group to be invalidated as its object no longer exists. Checking "if(function)" does not work since that checks 
	//if func is not empty, but it could be not empty and have invalid this pointer leading to crashes/strange behavior. A check for this must be implmeneted in the
	//event class to prevent this issue such as creating a function class wrapper
	//TODO: make optimizations based on suggestions in video: https://www.youtube.com/watch?v=IroPQ150F6c&t=1406s
	//TODO: merge vec2 and vec2int into one type with 2 template args that can be int,int for vec2int and float, float for vec2
	//TODO: finish making out arg class to simplify out parameters
	//TODO: a possible consideration to increase editor performance is to make all fields that are set into a wrapper such as SerializedField<T> so that we can track when a value is changed internally
	//and thus we can then update the gui element with the value, similar to how gui receive events on their update when to set internal value. This way, we do not force update every frame
	//TODO: add a tooltip system when hovering over elements that builds on the popup system already in place for gui systems
	//TODO: instead of doing mouse button, mouse left button, abstract into a event profile for special bindings for each type of action
	//TODO: make certain components require other components before they are added via entity data
	//TODO: check which is faster: getting global pos for transform via parent/entitydata recursion OR getting pointer to parent global pos after entity is set
	//TODO: since entity data is a component it has the ability to retrieve entity (aka itself) but it is NEVER set and can be nullptr. Therefore we must find a way for all components to either get an interface
	//that requires the get entity data safe function to be implemented because we CANNOT allow invalid function to exist on entity data
	//TODO: in order to allow similar caching of transform, maybe make gui element inherit from transform so that we can use it from entity
	//TODO: make a place, whether static storage or some registry that sotres component dependencies, what other components are required
	//TODO: optimize searching/non contiguous traversal of entities/components such as within guihierarchy by reserved ids within certain areas for faster lookup, entity data children such as binary search
	//for the children ids and others
	//TODO: since global pos is acceseed frequentyl figure out optimization (preferablly on components themselves) so they can access global pos without doing recursion
	//TODO: extracting int/float consider case where number is out of range
	//TODO: add ui transform as a gettable component from any other ui component
	//TODO: instead of doing component required check and then post add action of setting dependencies, we should probably instead in all component systems just make sure it has component (and add if missing) 
	//as well as wset the private reference then in one go rather than split in two functions
	//TODO: ensure all components hve default constructors so they can be made with no args when deserializing. All dependencies should be set in the system on the postadd event
	//TODO: remove components vector from entity data since you can just access componetns using registry pointer
	//TODO: make errors (and asserts) invoke debugger break/file line/function name for easier debugging
	//TODO: when doing required ocmponent checks it does not work with base types, meaning UIINput field which inherits from uiselectable will not do the required check for the base type
	//so instead we should make uiselectable(uiinterable) into separate component
	//TODO: make globals perhaps into a class/structure
	//TODO: abstract out all engine ui stuff into separate ui manager class
	//TODO: since there may be systems that have flags that are true for one update loop (to avoid events) make a separate object that does this and resets at the end of eveyr update loop
	//so it can be easily reused and so systems like uirenderer and ui interation manager do not need events for add element
	//TODO: right now ui renderer and transform have their own last frame rect vars, but most of the time renders are thesame for both, so ui renderer should probably use the var result from get function from transform
	//in order to prevent var storage duplication/ remove the return value from render

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

	constexpr LoopStatusCode SUCCESS_CODE = 0;
	constexpr LoopStatusCode EXIT_CODE = 1;
	constexpr LoopStatusCode ERROR_CODE = 2;

	void Engine::Destroy()
	{
		CloseWindow();
		EngineLog("DESTROYED ENGINE");
	}

	Engine::Engine() :
		m_engineState(),
		m_assetManager(),
		m_globalInitializer(m_assetManager),
		m_collisionRegistry(),
		m_sceneManager(m_assetManager),
		m_inputManager(m_assetManager),
		m_cameraController(),
		m_physicsManager(m_sceneManager, m_collisionRegistry),
		m_UIInteractionManager(m_inputManager, m_uiHierarchy),
		m_uiHierarchy(m_sceneManager.m_GlobalEntityManager, Vec2Int{SCREEN_WIDTH, SCREEN_HEIGHT}),
		m_popupManager(m_uiHierarchy),
		m_renderer(),
		m_transformSystem(),
		m_entityRendererSystem(),
		m_lightSystem(m_entityRendererSystem),
		//m_inputSystem(m_inputManager),
		m_spriteAnimatorSystem(m_entityRendererSystem),
		m_animatorSystem(),
		m_collisionBoxSystem(m_collisionRegistry),
		m_physicsBodySystem(m_physicsManager),
		m_playerSystem(m_inputManager),
		m_cameraSystem(m_renderer, &(m_collisionBoxSystem.GetColliderBufferMutable()), &(m_physicsBodySystem.GetLineBufferMutable())),
		m_particleEmitterSystem(),
		m_triggerSystem(),
		m_uiSystemExecutor(m_engineState, m_renderer, m_uiHierarchy, m_popupManager),
		//m_playerInfo(std::nullopt),
		//m_mainCameraInfo(std::nullopt),
		m_timeKeeper(),
		m_editor(m_timeKeeper, m_inputManager, m_physicsManager, m_assetManager,
			m_sceneManager, m_cameraController, m_UIInteractionManager, m_uiHierarchy, m_popupManager, m_collisionBoxSystem),
		m_gameManager(m_uiHierarchy)

	{
		EngineLog("FINISHED SYSTEM CONSTRUCTORS");

		//Note: input relies on assets, and 
		//asset manager needs to setup assets AFTER static global asset ref is set
		m_assetManager.Init();

		m_inputManager.Init();
		m_inputManager.SetInputCooldown(0.3);

		InitJsonSerializationDependencies(m_sceneManager, m_assetManager);
		m_assetManager.InitDependencies<SceneAsset, GlobalEntityManager, AssetManagement::AssetManager>(m_sceneManager.m_GlobalEntityManager, m_assetManager);
		m_assetManager.InitDependencies<InputProfileAsset, Input::InputManager>(m_inputManager);

		//TODO: glocal color codes should not reside in engine init but should be a second-class/hierarchy call
		GlobalColorCodes::InitCodes(m_assetManager);
		Templates::Init(m_assetManager);
		
		m_uiHierarchy.Init();
		m_popupManager.Init();
		m_uiSystemExecutor.Init();
		GlobalEntityCreator::CreateGlobals(m_sceneManager.m_GlobalEntityManager, m_sceneManager, m_cameraController, m_assetManager);

		//NOTE: we have to load all scenes AFTER all globals are created so that scenes can use globals for deserialization
		//if it is necessary for them (and to prevent misses and potential problems down the line)
		m_sceneManager.LoadAllScenes();
		//TODO: find a way to do this more procedurally
		m_sceneManager.m_OnSceneChange.AddListener([this](Scene* scene) -> void {StartAll(); });
		EngineLog("LOADED ALL SCENES");

		if (!Assert(m_sceneManager.TrySetActiveScene(0),
			std::format("Tried to set the active scene to the first one, but failed!")))
			return;

		EngineLog(std::format("SET FIRST SCENE:{}", m_sceneManager.GetActiveScene()->ToString()));
		//m_sceneManager.GetActiveSceneMutable()->InitScene();
		//m_sceneManager.GetActiveSceneMutable()->SetMainCamera(mainCameraEntity);
		//LogError(std::format("Scene active: {}", m_sceneManager.GetActiveScene()->ToString()));
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

		/*EngineLog(std::format("LOADED LAYERS FOR SCENE '{}': {}",
			m_sceneManager.GetActiveScene()->GetName(),
			m_sceneManager.GetActiveScene()->ToStringLayers()));*/

		//Log(std::format("OBSACLE ID: {}", obstacle.ToString()));
		//Log(std::format("CAMERA ID: {}", mainCameraEntity.ToString()));
		//Log(std::format("PLAYER ID: {}", playerEntity.ToString()));

		//m_inputManager.InitProfiles();
		
		//InitConsoleCommands();
		m_editor.Init(m_playerSystem);
		EngineLog("ADDED ALL CONSOLE COMMANDS");

		//Note: globals create main menu camera that then adds itself to each scene when scene is loaded
		//TODO: change this weird and akward way of setting camera that feels hidden

		//Assert(false, std::format("FOUND ACTIVE SELECTED: {}", m_UIInteractionManager.TryGetSelectableSelected()->GetLastFrameRect().ToString()));
		/*for (const auto& entity : m_sceneManager.GetActiveSceneMutable()->GetAllEntities())
		{
			LogError(std::format("Entity: {} has fields: {}", entity->m_Name,
				Utils::ToStringIterable<std::vector<ComponentField>, ComponentField>(entity->m_Transform.GetFields())));
		}*/
		m_gameManager.GameStart();
		EngineLog("FINISHED GAME INIT");

		//Note: gui selector init has to be after any other calls because we want to ensure we create the selectable
		//tree after as much time is given to add any gui elements to the ui tree
		m_UIInteractionManager.Init();

		m_engineState.SetExecutionState(ExecutionState::Validation);
		ValidateAll();

		//LogWarning(std::format("Tree hierarcxhy: {}", m_uiHierarchy.ToStringTree()));
		StartAll();
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
		m_gameManager.GameValidate();
		EngineLog("FINISHED VALIDATION");
	}
	void Engine::StartAll()
	{
		Scene* newScene = m_sceneManager.GetActiveSceneMutable();
		if (!Assert(newScene != nullptr, "Tried to call start on all systems but there "
			"are no scenes set as active right now"))
			return;
	}

	void Engine::EngineLog(const std::string& log) const
	{
		if (!DO_ENGINE_LOGS) return;
		LogMessage(LogType::Log, CallerLogDetails::None, log, false, true, ANSI_COLOR_BLUE, false);
	}

	LoopStatusCode Engine::Update()
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("Engine::Update");
#endif 
		LogWarning(std::format("FPS:{}", GetFPS()));

		m_timeKeeper.UpdateTimeStart();
		const float scaledDeltaTime = m_timeKeeper.GetLastScaledDeltaTime();
		const float unscaledDeltaTime = m_timeKeeper.GetLastIndependentDeltaTime();
		//LogError(std::format("Update scaled dt:{} unscaled:{} scale:{}", scaledDeltaTime, unscaledDeltaTime, m_timeKeeper.GetTimeScale()));

		m_inputManager.Update(unscaledDeltaTime);

		const FragmentedTextBuffer* frameBuffer = nullptr;
		if (m_editor.IsInGameView())
		{
			Scene* activeScene = m_sceneManager.GetActiveSceneMutable();
			if (!Assert(activeScene != nullptr, "Tried to update the active scene but there "
				"are none set as active right now"))
				return ERROR_CODE;

			if (!Assert(activeScene->HasEntities(), std::format("Tried to update the active scene:{} but there "
				"are no entities in the scene", activeScene->GetName())))
				return ERROR_CODE;

			m_cameraController.UpdateActiveCamera();
			CameraData& mainCamera = m_cameraController.GetActiveCameraMutable();

			std::string cameraSceneName = mainCamera.GetEntity().m_SceneName;
			if (!Assert(cameraSceneName == EntityData::GLOBAL_SCENE_NAME || cameraSceneName == activeScene->GetName(),
				std::format("Tried to get active camera:{} during update loop, "
					"but that camera is not in the active scene OR global storage (main camera scene:{}, active scene:{})", mainCamera.ToString(),
					cameraSceneName, activeScene->GetName())))
				return ERROR_CODE;

			//TODO: maybe some general scene stuff should be abstracted into scene manager
			activeScene->ResetAllLayers();
			activeScene->ResetFrameDirtyComponentCount();

			//TODO: ideally the systems would be supplied with only relevenat components without the need of entities
			//but this can only be the case if data is stored directyl without std::any and linear component data for same entities is used

			//Note: technically transform system should be using scaled time but since it is possible to change pos
			//even when time is stopped we need to make sure it updates just in case
			m_transformSystem.SystemUpdate(*activeScene, mainCamera, unscaledDeltaTime);
			//m_uiSystem.SystemUpdate(*activeScene, mainCamera, unscaledDeltaTime);

			m_playerSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
			m_collisionBoxSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
			m_physicsManager.GetPhysicsWorldMutable().UpdateStart(scaledDeltaTime);
			m_physicsBodySystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
			m_triggerSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
			m_animatorSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
			m_spriteAnimatorSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
			m_particleEmitterSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
			m_entityRendererSystem.SystemUpdate(*activeScene, mainCamera, unscaledDeltaTime);
			m_lightSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);
			m_cameraSystem.SystemUpdate(*activeScene, mainCamera, unscaledDeltaTime);
			m_gameManager.GameUpdate();

			m_UIInteractionManager.Update();
			//m_uiHierarchy.UpdateAll(unscaledDeltaTime);
			m_editor.Update(unscaledDeltaTime, scaledDeltaTime, m_timeKeeper.GetTimeScale());
			m_uiSystemExecutor.SystemsUpdate(m_sceneManager.m_GlobalEntityManager, unscaledDeltaTime);
			/*Rendering::RenderBuffer(frameBuffer, m_cameraSystem.GetCurrentColliderOutlineBuffer(),
				m_cameraSystem.GetCurrentLineBuffer(), &m_uiTree);*/
			m_renderer.RenderBuffer();

			m_transformSystem.UpdateLastFramePos(*activeScene);
		}
		else
		{
			m_UIInteractionManager.Update();
			//m_uiHierarchy.UpdateAll(unscaledDeltaTime);
			m_editor.Update(unscaledDeltaTime, scaledDeltaTime, m_timeKeeper.GetTimeScale());
			m_uiSystemExecutor.SystemsUpdate(m_sceneManager.m_GlobalEntityManager, unscaledDeltaTime);

			//Rendering::RenderBuffer(nullptr, nullptr, nullptr, &m_uiTree);
			m_renderer.RenderBuffer();
		}
		
		m_timeKeeper.UpdateTimeEnd();
		if (m_timeKeeper.ReachedFrameLimit()) 
			return EXIT_CODE;

		return SUCCESS_CODE;
	}

	void Engine::BeginUpdateLoop()
	{
		LoopStatusCode currentCode = SUCCESS_CODE;
		m_engineState.SetExecutionState(ExecutionState::Update);
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
				LogError(std::format("Update loop terminated due to exception: {}", e.what()), true, false);
				return;
			}

#ifdef ENABLE_PROFILER
			ProfilerTimer::m_Profiler.LogCurrentRoundTimes();
#endif
			if (!Assert(currentCode != ERROR_CODE, 
				std::format("Update loop terminated due to error"), true)) 
				return;

			if (!Assert(currentCode != EXIT_CODE,
				std::format("Update loop terminated due to loop end triggered"), true)) 
				return;
		}
	}
}
