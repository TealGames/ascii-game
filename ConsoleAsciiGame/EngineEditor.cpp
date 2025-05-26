#include "pch.hpp"
#include "EngineEditor.hpp"
#include "TimeKeeper.hpp"
#include "PlayerSystem.hpp"
#include "CameraController.hpp"
#include "PositionConversions.hpp"
#include "SceneManager.hpp"
#include "PhysicsManager.hpp"
#include "RaylibUtils.hpp"
#include "GUISelectorManager.hpp"
#include "CollisionBoxSystem.hpp"
#include "ColorPopupGUI.hpp"
#include "EditorStyles.hpp"

static constexpr float TOP_BAR_HEIGHT = 0.03;
static constexpr float ASSET_EDITOR_BUTTON_WIDTH = 0.2;
static constexpr float TOGGLE_LAYOUT_WIDTH_PER_TOGGLE = 0.05;
static const NormalizedPosition MOUSE_POS_TEXT_SIZE = {0.1, 0.05};

static constexpr KeyboardKey PAUSE_TOGGLE_KEY = KEY_P;
static constexpr float HELD_TIME_FOR_OBJECT_MOVE = 0.2;

EditModeInfo::EditModeInfo() : m_Selected(nullptr) {}

EngineEditor::EngineEditor(TimeKeeper& time, const Input::InputManager& input, Physics::PhysicsManager& physics, AssetManagement::AssetManager& assetManager,
	SceneManagement::SceneManager& scene, const CameraController& camera, GUISelectorManager& selector, GUIHierarchy& guiTree, ECS::CollisionBoxSystem& collisionSystem)
	:
	m_editorObj(nullptr),
	m_displayingGameView(true),
	m_timeKeeper(time), m_inputManager(input), m_sceneManager(scene), m_cameraController(camera),
	m_physicsManager(physics), m_guiSelector(selector), m_guiTree(guiTree), m_collisionBoxSystem(collisionSystem),
	m_commandConsole(m_inputManager, m_guiTree, m_guiSelector), m_debugInfo(guiTree),
	m_popupManager(guiTree),
	m_entityEditor(m_inputManager, m_cameraController, m_guiTree, m_popupManager, assetManager),
	m_spriteEditor(m_guiTree, m_inputManager, assetManager),
	m_overheadBarContainer(EditorStyles::EDITOR_BACKGROUND_COLOR), m_toggleLayout(LayoutType::Horizontal, SizingType::ShrinkOnly, {}),
	m_pauseGameToggle(false, EditorStyles::GetToggleStyle()), m_editModeToggle(false, EditorStyles::GetToggleStyle()),
	m_editModeInfo(), m_assetEditorButton(EditorStyles::GetButtonStyle(TextAlignment::Center), "AssetEditors"),
	m_mousePosText("", EditorStyles::GetTextStyleFactorSize(TextAlignment::Center))
{

	//scene.m_GlobalEntityManager.CreateGlobalEntity("__Editor", TransformData());

	/*const GUIStyle toggleSettings = GUIStyle( EntityEditorGUI::EDITOR_SECONDARY_COLOR,
		TextGUIStyle(EntityEditorGUI::EDITOR_TEXT_COLOR, FontProperties(0, EntityEditorGUI::EDITOR_CHAR_SPACING.m_X, GetGlobalFont()), 
			TextAlignment::Center, GUIPadding(), 0.8));

	const GUIStyle buttonSettings = GUIStyle( EntityEditorGUI::EDITOR_PRIMARY_COLOR,
		TextGUIStyle(EntityEditorGUI::EDITOR_TEXT_COLOR, FontProperties(0, EntityEditorGUI::EDITOR_CHAR_SPACING.m_X, GetGlobalFont()),
			TextAlignment::Center, GUIPadding(), 0.8));*/

	m_mousePosText.SetSize(MOUSE_POS_TEXT_SIZE);
	
	//m_pauseGameToggle.SetSettings(toggleSettings);
	m_editModeToggle.SetFixed(false, true);
	m_editModeToggle.SetValueSetAction([this](const bool isChecked)-> void
		{
			if (isChecked) m_guiTree.AddToRoot(DEFAULT_LAYER, &m_mousePosText);
			else
			{
				m_guiTree.TryRemoveElement(m_mousePosText.GetId());
				m_mousePosText.SetText("");
			}
		});

	m_pauseGameToggle.SetFixed(false, true);
	m_pauseGameToggle.SetValueSetAction([this](const bool isChecked) -> void
		{ 
			if (isChecked) m_timeKeeper.StopTimeScale();
			else m_timeKeeper.ResetTimeScale();

			//DebugProperties::SetLogMessages(!isChecked);
		});

	//m_editModeToggle.SetSettings(toggleSettings);
	m_toggleLayout.AddLayoutElement(&m_pauseGameToggle);
	m_toggleLayout.AddLayoutElement(&m_editModeToggle);

	const float layoutWidth = TOGGLE_LAYOUT_WIDTH_PER_TOGGLE * m_toggleLayout.GetChildCount();
	const float layoutStartX = (1 - layoutWidth) / 2;
	m_toggleLayout.SetBounds({ layoutStartX, 1}, {layoutStartX+ layoutWidth, 0});

	//m_assetEditorButton.SetSettings(buttonSettings);
	m_assetEditorButton.SetBounds(NormalizedPosition::TOP_LEFT, { ASSET_EDITOR_BUTTON_WIDTH, 0 });
	m_assetEditorButton.SetClickAction([this](const ButtonGUI& button)-> void
		{
			m_displayingGameView = !m_displayingGameView;
			if (m_displayingGameView)
			{
				m_assetEditorButton.SetText("AssetEditors");
				m_entityEditor.TryCloseCurrentEntityGUI();
			}
			else m_assetEditorButton.SetText("GameView");
		});

	m_overheadBarContainer.SetBounds(NormalizedPosition::TOP_LEFT, { 1, 1 - TOP_BAR_HEIGHT });
	m_overheadBarContainer.PushChild(&m_assetEditorButton);
	m_overheadBarContainer.PushChild(&m_toggleLayout);
	m_guiTree.AddToRoot(DEFAULT_LAYER, &m_overheadBarContainer);

	DebugProperties::OnMessageLogged.AddListener([this](const LogType& logType, const std::string& message, 
		const bool& logToConsole, const bool& pauseOnMessage)-> void
		{
			if (!pauseOnMessage) return;

			//To prevent paused going unpaused here we need it to be not paused state
			if (!m_pauseGameToggle.IsToggled()) 
				m_pauseGameToggle.ToggleValue();
		});

	//LogWarning(std::format("Popup addr:{}", Utils::ToStringPointerAddress(&m_popupManager)));
}

EngineEditor::~EngineEditor()
{
	m_commandConsole.DeletePrompts();
}

void EngineEditor::InitConsoleCommands(ECS::PlayerSystem& playerSystem)
{

	m_commandConsole.AddPrompt(new CommandPrompt<std::string, float, float>("setpos", { "EntityName", "PosX", "PosY" },
		[this](const std::string& entityName, const float& x, const float& y) -> void {
			if (ECS::Entity* entity = m_sceneManager.GetActiveSceneMutable()->TryGetEntityMutable(entityName, true))
			{
				entity->m_Transform.SetPos({ x, y });
			}
		}));

	m_commandConsole.AddPrompt(new CommandPrompt<std::string>("editor", { "EntityName" },
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
			m_commandConsole.LogOutputMessages(
				m_commandConsole.GetPromptDocumentationAll(), ConsoleOutputMessageType::Default);
		}));

	m_commandConsole.AddPrompt(new CommandPrompt<std::string>("debugmessage", std::vector<std::string>{"MessageFilter"},
		[](const std::string& messageFilter) -> void {
			DebugProperties::SetLogMessageFilter(messageFilter);
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

			DebugProperties::SetLogTypeFilter(maybeLogType.value());
			/*LogError(std::format("The new log type: {} has log: {}", LogTypeToString(GetLogTypeFilter()),
				std::to_string(Utils::HasFlagAny(GetLogTypeFilter(), LogType::Log))));*/
				//throw std::invalid_argument("POOP");
		}));

	m_commandConsole.AddPrompt(new CommandPrompt<>("debugreset", std::vector<std::string>{},
		[this]() -> void {
			DebugProperties::ResetLogFilters();
		}));

	m_commandConsole.AddPrompt(new CommandPrompt<float>("settimescale", std::vector<std::string>{"TimeScale"},
		[this](const float& scale) -> void {
			m_timeKeeper.SetTimeScale(scale);
		}));

	/*m_commandConsole.AddPrompt(new CommandPrompt<int>("debugmark", std::vector<std::string>{"Index"},
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
		}));*/

	m_commandConsole.AddPrompt(new CommandPrompt<bool>("cheats", std::vector<std::string>{"CheapStatus"},
		[&playerSystem](const bool& enableCheats) -> void {
			playerSystem.SetCheatStatus(enableCheats);
		}));

	m_commandConsole.AddPrompt(new CommandPrompt<>("save", std::vector<std::string>{},
		[this]() -> void {
			m_sceneManager.SaveCurrentScene();
		}));
}

void EngineEditor::SelectEntityEditor(ECS::Entity& entity)
{
	m_entityEditor.SetEntityGUI(*m_editModeInfo.m_Selected);
	m_popupManager.CloseAllPopups();
}

void EngineEditor::Init(ECS::PlayerSystem& playerSystem)
{
	InitConsoleCommands(playerSystem);

	//Note: the init order matters because it creates the order that the objects are added to the selector
	m_popupManager.AddPopup(new ColorPopupGUI(m_inputManager));
}

void EngineEditor::Update(const float unscaledDeltaTime, const float scaledDeltaTime, const float timeStep)
{
	m_assetEditorButton.Update(unscaledDeltaTime);
	m_popupManager.UpdatePopups(unscaledDeltaTime);
	m_commandConsole.Update(scaledDeltaTime);

	if (!IsInGameView())
	{
		m_spriteEditor.Update(unscaledDeltaTime);
		return;
	}

	Scene* activeScene = m_sceneManager.GetActiveSceneMutable();
	if (!Assert(this, activeScene != nullptr, std::format("Tried to update the engine editor but "
		"there are no active scenes right now", activeScene->GetName())))
		return;

	const CameraData& mainCamera = m_cameraController.GetActiveCamera();

	//Assert(false, std::format("Entity editor update"));
	//m_commandConsole.Update();
	m_debugInfo.Update(unscaledDeltaTime, timeStep, *activeScene, m_inputManager, mainCamera);

	//m_pauseGameToggle.Update();
	if (m_inputManager.IsKeyPressed(PAUSE_TOGGLE_KEY))
	{
		//LogError("Toggle pause");
		m_pauseGameToggle.ToggleValue();
	}

	//m_editModeToggle.Update();
	//LogError(std::format("Is toggled:{} selected:{}", std::to_string(m_editModeToggle.IsToggled()), std::to_string(m_editModeInfo.m_Selected != nullptr)));

	Vec2 mouseClickedPos = m_inputManager.GetMousePosition();
	WorldPosition worldClickedPos = Conversions::ScreenToWorldPosition(mainCamera, ScreenPosition(mouseClickedPos.m_X, mouseClickedPos.m_Y));
	if (m_inputManager.GetInputKey(MOUSE_BUTTON_LEFT)->GetState().IsPressed())
	{
		auto entitiesWithinPos = m_collisionBoxSystem.FindBodiesContainingPos(*activeScene, worldClickedPos);
		if (!entitiesWithinPos.empty())
		{
			m_editModeInfo.m_Selected = &(entitiesWithinPos[0]->GetEntitySafeMutable());
			SelectEntityEditor(*m_editModeInfo.m_Selected);
		}
	}
	
	else if (m_editModeToggle.IsToggled())
	{
		//If we are in edit mode holding the down button (and not selected selectable this frame-> meaning click might correspond to selectable click not edit mode click) 
		// we can move the selected entity to that pos
		if (m_inputManager.GetInputKey(MOUSE_BUTTON_LEFT)->GetState().IsDownForTime(HELD_TIME_FOR_OBJECT_MOVE) &&
			!m_guiSelector.SelectedSelectableThisFrame() && m_editModeInfo.m_Selected != nullptr)
		{
			/*Assert(false, std::format("Is down for:{} needed:{}", std::to_string(m_inputManager.GetInputKey(MOUSE_BUTTON_LEFT)->GetState().GetCurrentDownTime()), 
			std::to_string(HELD_TIME_FOR_OBJECT_MOVE)));*/
			m_editModeInfo.m_Selected->m_Transform.SetPos(worldClickedPos);
		}

		const Vec2 mousePos = m_inputManager.GetMousePosition();
		const Vec2Int rootSize = m_guiTree.GetRootSize();
		const Vec2 mousePosNorm = Vec2(mousePos.m_X / rootSize.m_X, (rootSize.m_Y- mousePos.m_Y)/ rootSize.m_Y);
		m_mousePosText.SetText(mousePos.ToString(2));
		const Vec2 textSize = m_mousePosText.GetSize().GetPos();
		m_mousePosText.SetTopLeftPos({ mousePosNorm.m_X- (textSize.m_X/2), mousePosNorm.m_Y+ textSize.m_Y});
	}
	m_entityEditor.Update();
}

bool EngineEditor::IsInGameView() const
{
	return m_displayingGameView;
}

/*
bool EngineEditor::TryRender()
{
	if (IsInGameView())
	{
		//m_commandConsole.TryRender();
		//m_entityEditor.TryRender();
		m_debugInfo.TryRender();

		const int TOGGLE_SIZE = 20;
		m_pauseGameToggle.Render(RenderInfo({ SCREEN_WIDTH / 2,0 }, { TOGGLE_SIZE, TOGGLE_SIZE }));
		m_editModeToggle.Render(RenderInfo({ SCREEN_WIDTH / 2 + TOGGLE_SIZE, 0 }, { TOGGLE_SIZE, TOGGLE_SIZE }));

		if (m_editModeToggle.IsToggled() && m_editModeInfo.m_Selected != nullptr)
		{
			const float RAY_LENGTH = 25;
			const ScreenPosition entityScreenPos = Conversions::WorldToScreenPosition(m_cameraController.GetActiveCamera(),
				m_editModeInfo.m_Selected->m_Transform.GetPos());

			const ScreenPosition axisRayXEndPos = entityScreenPos + ScreenPosition(RAY_LENGTH, 0);
			const ScreenPosition axisRayYEndPos = entityScreenPos + ScreenPosition(0, -RAY_LENGTH);
			//Assert(false, std::format("Rendering rays at pos:{}", entityScreenPos.ToString()));

			RaylibUtils::DrawRay2D(entityScreenPos, Vec2(RAY_LENGTH, 0), WHITE);
			RaylibUtils::DrawRay2D(entityScreenPos, Vec2(0, -RAY_LENGTH), WHITE);

			//DrawLine(entityScreenPos.m_X, entityScreenPos.m_Y, axisRayXEndPos.m_X, axisRayXEndPos.m_Y, BLUE);
			//DrawLine(entityScreenPos.m_X, entityScreenPos.m_Y, axisRayYEndPos.m_X, axisRayYEndPos.m_Y, BLUE);
		}
	}
	else
	{
		const float leftPadding = 150;
		DrawRectangle(0, 0, leftPadding, SCREEN_HEIGHT, EditorStyles::EDITOR_BACKGROUND_COLOR);

		DrawRectangle(leftPadding, 0, SCREEN_WIDTH- leftPadding, SCREEN_HEIGHT, EditorStyles::EDITOR_SECONDARY_BACKGROUND_COLOR);
		//m_spriteEditor.Render(RenderInfo(ScreenPosition(leftPadding, 0), ScreenPosition(SCREEN_WIDTH - leftPadding, SCREEN_HEIGHT)));
	}

	m_assetEditorButton.Render(RenderInfo({ 0, 0 }, { 120, 20 }));

	//NOTE: popup manager MUST be the last editor GUI to be rendered
	//m_popupManager.RenderPopups();

	return true;
}
*/