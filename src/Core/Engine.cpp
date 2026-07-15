#include "pch.hpp"
#include "Core/Engine.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "Game/SceneCreator.hpp"
#include "StaticGlobals.hpp"
#include "Core/EngineLog.hpp"
#include "Core/Rendering/Renderer3D.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"
#include "ECS/Systems/Types/World/EntityRenderer2DSystem.hpp"
#include "ECS/Systems/Types/World/CameraSystem.hpp"
#include "ECS/Systems/Types/World/LightSource2DSystem.hpp"
#include "ECS/Systems/Types/World/AnimatorSystem.hpp"
#include "ECS/Systems/Types/World/SpriteAnimatorSystem.hpp"
#include "ECS/Systems/Types/World/PhysicsBodySystem.hpp"
#include "Core/UIElementTemplates.hpp"
#include "Core/Analyzation/ProfilerTimer.hpp"
#include "Core/Asset/InputProfileAsset.hpp"
#include "Core/Serialization/SerializationUtils.hpp"
#include "Game/GlobalCreator.hpp"
#include "Core/Asset/GlobalColorCodes.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/World/PointLight3DComponent.hpp"
#include "Math/Math3d.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "Core/Asset/Model3dAsset.hpp"


namespace Engine::Core
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
	//TODO: for some speedup, remove std::format() from if condition and instead inside if scope to reduce overhead of formatting strings when we do assets, or just simple checks, and only when it is necessary for logging wrong value
	//TODO: msot interaction components do not need update and should instead do things on interaction event, like select, drag, etc since really they need 
	//to be updated when getting an event not every single frame
	//TODO: on selectable, we allow calling hover start/end and click, which should only be possible with interaction manager because they are driven by input. select/dselect is fine
	//TODO: visual data has similar visual storing/creating functions so prevent the need for copying so much similar logic
	//TODO: what if instead of rendering each segment as text, what if they were all textures with transparent backgrounds and characters made in photshop -> making it easier to have
	//custom text arrangmenets
	//TODO: to improve performance since we create many vectors, it may be a good idea to create vector views instead of doing .xy to create new ones if we do not need
	//a new vector and only need to read, we can make views into existing memory and work with that especially if we only need components to create new vectors
	//TODO: make an interaction profile that sotres all the current interaction keycodes (like select = mousebuttomleft) and make design extensible so that you can choose
	//a new dvice and then it can have different keycode values. so maybe there is a default profile for each device and then when switching, a different one is activated
	//TODO: right now for shader asset (and scene asset) we avoid asset manager dependency by using static member function for setting asset hidden, which should not be allowed
	//TODO: consider making a shader asset that can create multiple shader programs (store all of them? or maybe create glsl files and then force asset generation?)
	//but it should be done by having one shader with ifdef macros and then defining them or not based on some flag or user command args that then get passed as multiple sources
	//into opengl when compiling shader
	//TODO: make camera system precalculated data update lazy and should only update when a value for it updates (view matrix-> transform updates, proj matrix-> setting updates)
	//TODO: ideally instead of checking each frame if a shader needs uniform buffer, we would have centralzied assigner that goes through every shader, looks through
	//every possible uniform block and ifnds the correct one it needs based on a registry
	//TODO: by default press -> down key state change is driving by os time delay (usually 0.5 s) which can be annoying so implement custom timer for customizable times
	//TODO: instead of querying for profile by name all the time make input system bind/unbind profiles and each profile can have many different settigns in addition to keybinds
	//like input delays, and other stuff so data can easily be changed via profiles and not via some manager
	//TODO: implement a proper dirty system where dirty components can then set dependent components and their flags (because if you just check a flag on the dependent component
	//the flag may be false due to a race for who calls function that lazily updates first). Hybrid approach: use event callbacks for low frequency components like camera/transfrom
	//where component may store some space for callbacks for flags, which then can be set via the dependent component changing its own dirty flag after the dependent component invokes
	//the notification. for larger system pairs like physics, rendering, transform etc. then use a registry that for each entity id, the respective pointer to flag that needs to change
	//the new value of the flag, and what flag it is targeting. and then the issuing/dependent component can issue registry check which would check its entity id and essentially
	//set all of flags in that moment. the dependening components can simply just register its own data and then when depedent component 
	// gets dirty -> checks registry -> updates its own entity flags
	//TODO: renderer updates: make instanced truly instanced -> if you call multiple spheres, only one sphere vertex data should be added to buffer, so next draw call
	//would just add instance data (remember instance calls cycle through given instance count for each each instance). Also while it is fine to do vertex attributes for 
	//instanced data, for more robustness, and to allow for increasing sizes of instances -> add instances to a uniform buffer (just store each instance property as an array in
	//the buffer block in the shader) and then when using instnace calls you can use gl_instanceId for the index of the instance buffer in shader (so just do uInstanceBlock[gl_instanceId])
	//and if you want use baseInstance draw call instead to then have base instance offsets if you want to store instnaced data for all calls in one buffer.
	//Also, for future create a indrect command buffer that allows for commands to be put into a buffer, allowing for more flexibility and batching multiple draws in one
	//api call using drawIndirectMulti. this is most usefl when things get more complicated and you want finer control over the process
	//Also, buffers who are dynamic with many updates should probably do bufferrange and mapbuffer to get pointer to memory that is always allocated for writing
	//instead of doing map and unmap every time which can be slow
	//TODO: remove all unnecessary color constructors especially int and std::uint8_t for floating types as they should just divide by 255 if you dont want to 
	//write out the conversion or explicit constructor
	//TODO: replace all instances of comparing types with typeid with constexpr is same type trait
	//TODO: add optimizations for debug builds so that on DEBUG macro, things like string functions, debug operations, etc are not included in build
	//TODO: add SIMD for vector, matrices, quaternions AND optimize them to run as fast as possible, esepcially by making sure
	//expensive operations like matrix multiplication runs as fast as possible by utilizing doing operations on data close together
	//TODO: optimize headers more to reduce rebuild times (put all stable related things into one header, like maybe put all vector types into one header
	//since they are often used together and should rarely change, maybe move all to stirng function into separate header)
	//TODO: add parallelization/concurency especially for expensive operations like physics, rendering
	//TODO: add support for negative scale with BVH by converting negative scale to rotation during the calculate model matrix part
	//TODO: for every single update fro every system we create a new function which invokes the update for all objects. instead make the function local
	//and on init or start let the system bind the update function on start so we invoke a function every time ratehr than create a new std::function
	//and/or functor object with this capture group which may be expensive
	//TODO: currently for immovable objects we just add draw call on start, which is bad because even though the vertex data might not change
	//because the object does not move, some instance data like material, texture might still change and would need an update
	//TODO; right now we import 3d shapes and assets to use for vertices from blender but some files might be big so it might be a good idea
	//to extract all vertex/normal/uv data and other vertex info and write to .txt and make our own file extension which can be read
	//TODO: there is a lot of string copies being made in fig which need to be fixed with string view
	//TODO: right now in renderer we cache materials based on their name which is BAD. if a material's name changes but keeps its data
	//it will get a new entry which means the old materials with theit previous names will accumulate. instead we want to keep material by id
	//and then check their id which should not change over the execution of the program
	//TODO: right now transform updates its global transform using recursion implicityl through a parent precalculated update call. change
	//to use loop and not recursion since recursion may be slow and take up a lot of memory
	//TODO: make it so that in debug assert can never be turned off so even if we ignore errors in debug settings, assert should always be invokved and shown
	//TODO: ideally we would make the Ui rect normalized pos change so TOP LEFT is (0,0) (NOT (0,1)) so y increases DOWN (not up) so we could do vector operations
	//without needing to flip operands for x and y components so we could get simd operation benefits
	//TODO: improve render unit so we do not need to clear indices/vertices every frame sine most of the time vertices will stay the same, and just batches
	//and instances will be different so we want to optimize for that
	//TODO; the vtx 3d model custom engine format takes up more space than fbx which defeats the whole point of a custom format. 
	//Optimize it more like removing unneeded uv storage AND ALSO SUPPORT FOR MATERIALS
	//TODO: right now we have some places where we do alloca when we want runtime sizes for arrays, instead to avoid using heap allocations we should make an arena
	//that allocates a lot of space upfront on the stack, and then we use it whenever we want that runtime size array and place it in there and we still get benefit 
	//of non-fragmented memory, pointers and runtime sizes
	//TODO; rihgt now in rendering we use window size for screen size which is not good because we support constrainted window sizes and keeping aspect ratios of the 
	//viewport area which differs from the actual window size. We now need to make it so the viewport area/aspect ratio constraint is based on the active camera settings
	//and as a result, the "screen size" value used in rendering should be based on the active camera viewport area NOT window size
	//TODO; right now for texture asset read/write to/from file we auto flip vertically since most file formats have different memory layout for textures
	//compared to OpenGL. the choice is to either match OpenGL is CPU texture buffers, which makes it simple to convert, but forces up to check if we need to flip
	//and may be problematic for other render APIs in the future which may not need flipping automatically if they store the texture differently. OR we could 
	//store like file formats and make it future proof, but also means we need to flip when going between GPU and CPU buffers which may become annoying and slow
	//TODO: REwrite render system:
	// 1) Make vertex layout (we call vertex layout, opengl calls it VertexArrayObject) have a separate Bind function
	//	  so that we can bind different layouts before draw so we can use different vertex/index/instance buffer pairs for different draw calls
	//	  since right now we assume all shaders must use the same vertex layouts which can be wasteful for things like debug draw calls which do not
	//	  need the vertices to have that must data. NOTE: LAYOUT MUST BE BOUND BEFORE ADDING ATTRIBUTES (MAKE SURE OPENGL IMPLEMENATION INTERNALLY
	//	  CHECKS THE VAO IS BOUND BEFORE ATTRIBUTES ARE ADDED SINCE OPENGL ASSOCIATES ATTRIBUTES BASED ON BOUND VAO) AND BEFORE CREATING INDEX BUFFER
	//	  (SINCE OPENGL IMPLICITLY LINKS AN INDEX BUFFER TO A VAO BASED ON WHICH IS BOUND UNLIKE A VERTEX BUFFER WHICH NEEDS TO BE EXPLICITLY BOUND)
	// 2) Create Render Unit which should ideally store geometry with SAME LAYOUT and should not really do much more except be a storage container
	//	  that contains the vertex layout, vertex, index and instance buffers cpu side as well as their gpu handle counterparts
	// 3) A render batch should instead store RenderUnit Pointer, the size and offset into vertex, index and instance buffers in that Unit. 
	//	  NOTE: each render batch is essnetially a separate draw call
	// 4) Each pass data should store a shader, any textures that need to be bound (good opporunity for optimization where objects can be part of same pass
	//	  even if they have different textures as long as it is less than the max amount of textures that can be bound) and all batch information, which 
	//	  must be contiguous. Also, it would need to store a map of batch hash to key indices in its batch list. 
	// 5) When we draw, we go through all passes. Each pass will then have all of its batches contigously. The textures/images 
	//	  associated with a pass will be bound, the shader will also be bound. NOTE: since we bind one shader per pass, there should be 
	//    *technically* the same vertex/index/instance for all draws and thus only one vao/vertex layout bound at the start (accessed via batch->renderUnit->layout)
	//	  For each batch in the pass, we will draw the segment from the vertex/index/instance buffers 
	// 6) When deciding what objects to add to what pass render calls now have more control to decide if those objects might need shadow, lighting
	//	  and based off of that, we add them to the corresponding pass or passes. Then we compute a batch hash based on color (like alpha which needs separate pass)
	//	  vertex count to determine the batch index it should be placed in the pass

	constexpr std::uint8_t TERMINATE_AFTTER_FRAMES = TimeKeeper::NO_FRAME_LIMIT;
	constexpr bool SHOW_FPS = true;

	constexpr std::streamsize DOUBLE_LOG_PRECISION = 8;

	//If true, even if the current output from camera is null, will render default data.
	//This is useful for testing the render loop and finding out FPS (since fps depends on drawing loop)
	constexpr bool ALWAYS_RENDER = true;
	//If true, will log all output from command contrller when executing commands
	constexpr bool DEBUG_LOG_COMMAND_OUTPUT = true;

	constexpr const char* START_SCENE_NAME = "scene1";

	void Engine::Destroy()
	{
		EngineLog("DESTROYED ENGINE");
	}

	Engine::Engine() :
		m_timeKeeper(TERMINATE_AFTTER_FRAMES),
		m_windowManager(), m_assetManager(), m_cameraController(),
		m_sceneManager(m_assetManager), m_graphicsManager(m_assetManager),
		m_inputManager(m_assetManager, m_windowManager),
		m_engineState(m_graphicsManager, m_assetManager, m_cameraController, m_inputManager, m_sceneManager, m_timeKeeper),
		m_collisionRegistry(), m_physicsManager(m_sceneManager, m_collisionRegistry),
		m_UIInteractionManager(m_inputManager, m_uiHierarchy), m_uiHierarchy(m_sceneManager.m_GlobalEntityManager),
		m_popupManager(m_uiHierarchy),
		m_renderer(m_engineState),
		m_transformSystem(),
		m_entityRendererSystem(m_renderer),
		m_lightSystem(m_entityRendererSystem),
		//m_inputSystem(m_inputManager),
		m_spriteAnimatorSystem(m_entityRendererSystem), m_animatorSystem(),
		m_meshSystem(m_renderer, m_engineState),
		m_collisionBoxSystem(m_collisionRegistry), m_physicsBodySystem(m_physicsManager),
		m_playerSystem(m_inputManager),
		m_cameraSystem(m_renderer),
		m_particleEmitterSystem(),
		m_triggerSystem(),
		m_uiSystemExecutor(m_engineState, m_renderer, m_uiHierarchy, m_popupManager),
		m_gizmosOverlay(m_uiSystemExecutor.m_UiRenderSystem, m_physicsManager, m_cameraController),
		m_commandController(DEBUG_LOG_COMMAND_OUTPUT),
		//m_playerInfo(std::nullopt),
		//m_mainCameraInfo(std::nullopt),
		m_editor(m_timeKeeper, m_inputManager, m_physicsManager, m_assetManager, m_sceneManager, m_cameraController, 
			m_UIInteractionManager, m_uiHierarchy, m_popupManager, m_collisionBoxSystem, m_commandController, m_gizmosOverlay),
		m_gameManager(m_uiHierarchy)
	{
		EngineLog("FINISHED SYSTEM CONSTRUCTORS");
		Serialization::InitSerializationUtils(m_sceneManager, m_assetManager);

		m_windowManager.m_OnWindowCreated.AddListener([this](Window* window)-> void 
			{
				m_engineState.m_GraphicsContext.m_Window = window;
				if (!m_renderer.WasInit()) m_renderer.Init();
			});
		m_windowManager.m_OnWindowUpdated.AddListener([this](Window* window)-> void 
			{
				SystemUpdate(*window); 
			});

		Window* createdWindow = m_windowManager.CreateNewWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ASPECT_RATIO, WINDOW_NAME, nullptr);
		if (createdWindow == nullptr || !createdWindow->IsValid())
		{
			LogError(std::format("Failed to create valid window"));
			return;
		}
		createdWindow->SetCursorMode(WindowCursorMode::Disabled);
		EngineLog("CREATED WINDOW");

		//Note: input relies on assets, and 
		//asset manager needs to setup assets AFTER static global asset ref is set
		std::filesystem::path assetPath = ASSET_DIR;
		m_assetManager.Init(m_engineState, assetPath);

		m_inputManager.Init();
		m_inputManager.SetInputCooldown(0.3);
	
		//TODO: glocal color codes should not reside in engine init but should be a second-class/hierarchy call
		GlobalColorCodes::InitCodes(m_assetManager);
		UI::Templates::Init(m_assetManager);
		
		m_uiHierarchy.Init();
		m_popupManager.Init();
		m_uiSystemExecutor.Init();
		m_spriteAnimatorSystem.Init();
		Scenes::GlobalEntityCreator::OnGlobalsInit(m_sceneManager.m_GlobalEntityManager, m_sceneManager, m_cameraController, m_assetManager);

		//NOTE: we have to load all scenes AFTER all globals are created so that scenes can use globals for deserialization
		//if it is necessary for them (and to prevent misses and potential problems down the line)
		m_graphicsManager.InitGraphicResources();
		Scenes::SceneCreator::Init(m_engineState);
		m_sceneManager.LoadAllSceneAssets();

		//TODO: find a way to do this more procedurally
		m_sceneManager.m_OnActiveSceneChange.AddListener([this](Scenes::Scene* scene) -> void {SystemStart(*scene); });
		EngineLog("LOADED ALL SCENES");

		if (!Assert(m_sceneManager.TrySetActiveScene(START_SCENE_NAME), "Tried to set the active scene to:{}, but failed!", START_SCENE_NAME))
			return;

		EngineLog("SET FIRST SCENE:{}", m_sceneManager.GetActiveScene()->ToString());
		EngineLog("SET FIRST SCENE CAMERA");

		m_editor.Init(m_playerSystem);
		EngineLog("ADDED ALL CONSOLE COMMANDS");

		//Note: globals create main menu camera that then adds itself to each scene when scene is loaded
		//TODO: change this weird and akward way of setting camera that feels hidden
		m_gameManager.GameStart();
		EngineLog("FINISHED GAME INIT");

		m_engineState.SetExecutionState(ExecutionState::Validation);
		SystemValidate();
	}

	Engine::~Engine()
	{
		//m_commandConsole.DeletePrompts();
	}

	void Engine::SystemValidate()
	{
		m_assetManager.Validate();
		m_sceneManager.ValidateAllScenes();
		m_cameraController.Validate();
		m_gameManager.GameValidate();
		EngineLog("FINISHED VALIDATION");
	}
	void Engine::SystemStart(Scenes::Scene& scene)
	{
		m_meshSystem.SystemStart(scene);
	}

	void Engine::SetUpdateStatusCode(const UpdateStatusCode& code)
	{
		m_engineState.m_LastUpdateStatus = code;
	}

	void Engine::SystemUpdate(Window& window)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("Engine::Update");
#endif 
		//LogWarning("UPDATE CALLED");

		m_timeKeeper.UpdateTimeStart();
		const float scaledDeltaTime = m_timeKeeper.GetLastScaledDeltaTime();
		const float unscaledDeltaTime = m_timeKeeper.GetLastIndependentDeltaTime();
		LogWarning(std::format("FPS:{}", 1 / unscaledDeltaTime));
		/*LogWarning(std::format("Update scaled dt:{} unscaled:{} scale:{} FPS (raylib):{} FPS(engine):{}", 
			scaledDeltaTime, unscaledDeltaTime, m_timeKeeper.GetTimeScale(), GetFPS(), 1/unscaledDeltaTime));*/

		m_inputManager.Update(unscaledDeltaTime);

		const Rendering::FragmentedTextBuffer2D* frameBuffer = nullptr;
		Scenes::Scene* activeScene = nullptr;
		if (m_editor.IsInGameView())
		{
			activeScene = m_sceneManager.GetActiveSceneMutable();
			if (!Assert(activeScene != nullptr, "Tried to update the active scene but there "
				"are none set as active right now"))
			{
				SetUpdateStatusCode(UpdateStatusCode::Error);
				return;
			}

			if (!Assert(activeScene->HasEntities(), "Tried to update the active scene:{} but there "
				"are no entities in the scene", activeScene->GetName()))
			{
				SetUpdateStatusCode(UpdateStatusCode::Error);
				return;
			}

			m_cameraController.UpdateActiveCamera();
			Camera::CameraComponent& mainCamera = m_cameraController.GetActiveCameraMutable();

			std::string cameraSceneName = mainCamera.GetEntity().m_SceneName;
			if (!Assert(cameraSceneName == ECS::EntityData::GLOBAL_SCENE_NAME || cameraSceneName == activeScene->GetName(),
				"Tried to get active camera:{} during update loop, "
					"but that camera is not in the active scene OR global storage (main camera scene:{}, active scene:{})", mainCamera.ToString(),
					cameraSceneName, activeScene->GetName()))
			{
				SetUpdateStatusCode(UpdateStatusCode::Error);
				return;
			}

			//TODO: maybe some general scene stuff should be abstracted into scene manager
			activeScene->ResetAllLayers();
			activeScene->ResetFrameDirtyComponentCount();

			//TODO: ideally the systems would be supplied with only relevenat components without the need of entities
			//but this can only be the case if data is stored directyl without std::any and linear component data for same entities is used

			//Note: technically transform system should be using scaled time but since it is possible to change pos
			//even when time is stopped we need to make sure it updates just in case
			m_transformSystem.SystemUpdate(*activeScene, mainCamera, unscaledDeltaTime);
			m_cameraSystem.SystemUpdate(*activeScene, mainCamera, unscaledDeltaTime);

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
			m_meshSystem.SystemUpdate(*activeScene, mainCamera, scaledDeltaTime);

			m_gameManager.GameUpdate();
		}

		m_editor.Update(unscaledDeltaTime, scaledDeltaTime, m_timeKeeper.GetTimeScale());
		m_UIInteractionManager.Update();
		m_uiSystemExecutor.SystemsUpdate(m_sceneManager.m_GlobalEntityManager, unscaledDeltaTime);

		m_gizmosOverlay.MoveCallsToRenderBuffer(m_renderer);

		/*const Mat4 modelMatrix = CalculateTranslationMatrix(objectCenter) * CalculateTranslationMatrix(Vec3::Zero()) * 
			CalculateRotationMatrix(rot) * CalculateTranslationatrix(-Vec3::Zero())  * CalculateScaleMatrix(Vec3::One());*/

		//LogWarning(std::format("Object rot is:{}", rot.ToDegrees().ToString()));
		//m_renderer.AddRectangleCall2D(Vec3(0, 0, 4.8), Vec2(0.13, 0.13), modelMatrix, Color_BLUE);
		
		//m_renderer.AddRectangleCall3D(Vec3(0.13, 0.13, 0.13), modelMatrix, Color_BLUE);
		//bool inView = m_cameraController.GetActiveCamera().DoesViewVolumeContainPos(Vec3(-10, 0, 0));
		//LogError(std::format("rectange oirign screen pos:{}", m_cameraController.GetActiveCamera().WorldToScreenPosition(Vec3(0, 0, 4.9)).ToString()));

		/*
		static float time = 0;
		static bool onLight = false;
		time += unscaledDeltaTime;
		if (time > 1)
		{
			if (onLight) m_renderer.ClearDirectionalLight();
			else 
			onLight = !onLight;
			time = 0;
		}
		*/
		
		m_renderer.RenderBuffer();

		//if (m_editor.IsInGameView())
		//{
		//	m_transformSystem.UpdateLastFramePos(*activeScene);
		//}
		
		m_uiHierarchy.Update();
		m_inputManager.UpdateEnd();
		m_timeKeeper.UpdateTimeEnd();
		if (m_timeKeeper.ReachedFrameLimit())
		{
			SetUpdateStatusCode(UpdateStatusCode::Exit);
			return;
		}

		SetUpdateStatusCode(UpdateStatusCode::Success);
	}

	void Engine::BeginUpdateLoop()
	{
		m_engineState.SetExecutionState(ExecutionState::Update);

		bool anyWindowActive = m_windowManager.GetActiveWindowCount()>0;
		while (anyWindowActive)
		{
			try
			{
				m_windowManager.UpdateAllWindows();
				anyWindowActive = m_windowManager.GetActiveWindowCount() > 0;
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Encountered window update error:{}", e.what()));
			}

			if (m_engineState.m_LastUpdateStatus == UpdateStatusCode::Error)
			{
				LogError(std::format("Update loop terminated due to error"));
				return;
			}
			else if (m_engineState.m_LastUpdateStatus == UpdateStatusCode::Exit)
			{
				LogError(std::format("Update loop terminated due to loop end triggered"));
				return;
			}
		}
	}
}
