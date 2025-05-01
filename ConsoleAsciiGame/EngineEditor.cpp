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

static constexpr KeyboardKey PAUSE_TOGGLE_KEY = KEY_P;
static constexpr float HELD_TIME_FOR_OBJECT_MOVE = 0.2;

EditModeInfo::EditModeInfo() : m_Selected(nullptr) {}

EngineEditor::EngineEditor(TimeKeeper& time, const Input::InputManager& input, Physics::PhysicsManager& physics, AssetManagement::AssetManager& assetManager,
	SceneManagement::SceneManager& scene, const CameraController& camera, GUISelectorManager& selector, ECS::CollisionBoxSystem& collisionSystem)
	: 
	m_displayingGameView(true),
	m_timeKeeper(time), m_inputManager(input), m_sceneManager(scene), m_cameraController(camera),
	m_physicsManager(physics), m_guiSelector(selector), m_collisionBoxSystem(collisionSystem),
	m_commandConsole(m_inputManager, selector), m_debugInfo(), 
	m_popupManager(),
	m_entityEditor(m_inputManager, camera, selector, m_popupManager),
	m_spriteEditor(selector, input, assetManager),
	m_pauseGameToggle(selector, false, GUISettings()), m_editModeToggle(selector, false, GUISettings()),
	m_editModeInfo(), m_assetEditorButton(selector, GUISettings(), "AssetEditors")
{
	const GUISettings toggleSettings = GUISettings({ 20, 20 }, EntityEditorGUI::EDITOR_SECONDARY_COLOR,
		TextGUISettings(EntityEditorGUI::EDITOR_TEXT_COLOR, FontProperties(0, EntityEditorGUI::EDITOR_CHAR_SPACING.m_X, GetGlobalFont()), 
			TextAlignment::Center, GUIPadding(), 0.8));

	const GUISettings buttonSettings = GUISettings({ 20, 20 }, EntityEditorGUI::EDITOR_PRIMARY_COLOR,
		TextGUISettings(EntityEditorGUI::EDITOR_TEXT_COLOR, FontProperties(0, EntityEditorGUI::EDITOR_CHAR_SPACING.m_X, GetGlobalFont()),
			TextAlignment::Center, GUIPadding(), 0.8));

	m_pauseGameToggle.SetSettings(toggleSettings);
	m_pauseGameToggle.SetValueSetAction([this](const bool isChecked) -> void
		{ 
			if (isChecked) m_timeKeeper.StopTimeScale();
			else m_timeKeeper.ResetTimeScale();

			//DebugProperties::SetLogMessages(!isChecked);
		});

	m_editModeToggle.SetSettings(toggleSettings);

	m_assetEditorButton.SetSettings(buttonSettings);
	m_assetEditorButton.SetClickAction([this](const ButtonGUI& button)-> void
		{
			m_displayingGameView = !m_displayingGameView;
			if (m_displayingGameView) m_assetEditorButton.SetText("AssetEditors");
			else m_assetEditorButton.SetText("GameView");
		});

	DebugProperties::OnMessageLogged.AddListener([this](const LogType& logType, const std::string& message, 
		const bool& logToConsole, const bool& pauseOnMessage)-> void
		{
			if (!pauseOnMessage) return;

			//To prevent paused going unpaused here we need it to be not paused state
			if (!m_pauseGameToggle.IsToggled()) 
				m_pauseGameToggle.ToggleValue();
		});

	LogWarning(std::format("Popup addr:{}", Utils::ToStringPointerAddress(&m_popupManager)));
	m_popupManager.AddPopup(new ColorPopupGUI(m_guiSelector, m_inputManager), { 100, 100 }, HIGHEST_PRIORITY);
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
			m_commandConsole.LogOutputMessagesUnrestricted(
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
		[&playerSystem](const bool& enableCheats) -> void {
			playerSystem.SetCheatStatus(enableCheats);
		}));

	m_commandConsole.AddPrompt(new CommandPrompt<>("save", std::vector<std::string>{},
		[this]() -> void {
			m_sceneManager.SaveCurrentScene();
		}));
}

void EngineEditor::Init(ECS::PlayerSystem& playerSystem)
{
	InitConsoleCommands(playerSystem);
}

void EngineEditor::Update(const float deltaTime, const float timeStep)
{
	m_assetEditorButton.Update(deltaTime);
	m_popupManager.UpdatePopups(deltaTime);

	if (!IsInGameView())
	{
		m_spriteEditor.Update(deltaTime);
		return;
	}

	Scene* activeScene = m_sceneManager.GetActiveSceneMutable();
	if (!Assert(this, activeScene != nullptr, std::format("Tried to update the engine editor but "
		"there are no active scenes right now", activeScene->GetName())))
		return;

	const CameraData& mainCamera = m_cameraController.GetActiveCamera();

	//Assert(false, std::format("Entity editor update"));
	m_commandConsole.Update();
	m_debugInfo.Update(deltaTime, timeStep, *activeScene, m_inputManager, mainCamera);

	m_pauseGameToggle.Update();
	if (m_inputManager.IsKeyPressed(PAUSE_TOGGLE_KEY))
	{
		//LogError("Toggle pause");
		m_pauseGameToggle.ToggleValue();
	}

	m_editModeToggle.Update();
	//LogError(std::format("Is toggled:{} selected:{}", std::to_string(m_editModeToggle.IsToggled()), std::to_string(m_editModeInfo.m_Selected != nullptr)));

	ScreenPosition mouseClickedPos = m_inputManager.GetMousePosition();
	WorldPosition worldClickedPos = Conversions::ScreenToWorldPosition(mainCamera, mouseClickedPos);
	if (m_inputManager.GetInputKey(MOUSE_BUTTON_LEFT)->GetState().IsPressed())
	{
		auto entitiesWithinPos = m_collisionBoxSystem.FindBodiesContainingPos(*activeScene, worldClickedPos);
		if (!entitiesWithinPos.empty())
		{
			m_editModeInfo.m_Selected = &(entitiesWithinPos[0]->GetEntitySafeMutable());
			m_entityEditor.SetEntityGUI(*m_editModeInfo.m_Selected);
		}
	}
	//If we are in edit mode holding the down button (and not selected selectable this frame-> meaning click might correspond to selectable click not edit mode click) 
	// we can move the selected entity to that pos
	else if (m_editModeToggle.IsToggled() && m_inputManager.GetInputKey(MOUSE_BUTTON_LEFT)->GetState().IsDownForTime(HELD_TIME_FOR_OBJECT_MOVE) &&
		!m_guiSelector.SelectedSelectableThisFrame() && m_editModeInfo.m_Selected != nullptr)
	{
		/*Assert(false, std::format("Is down for:{} needed:{}", std::to_string(m_inputManager.GetInputKey(MOUSE_BUTTON_LEFT)->GetState().GetCurrentDownTime()), 
			std::to_string(HELD_TIME_FOR_OBJECT_MOVE)));*/
		m_editModeInfo.m_Selected->m_Transform.SetPos(worldClickedPos);
	}
	m_entityEditor.Update();
}

bool EngineEditor::IsInGameView() const
{
	return m_displayingGameView;
}

bool EngineEditor::TryRender()
{
	if (IsInGameView())
	{
		m_commandConsole.TryRender();
		m_entityEditor.TryRender();
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
		DrawRectangle(0, 0, leftPadding, SCREEN_HEIGHT, EntityEditorGUI::EDITOR_BACKGROUND_COLOR);

		DrawRectangle(leftPadding, 0, SCREEN_WIDTH- leftPadding, SCREEN_HEIGHT, EntityEditorGUI::EDITOR_SECONDARY_BACKGROUND_COLOR);
		m_spriteEditor.Render(RenderInfo(ScreenPosition(leftPadding, 0), ScreenPosition(SCREEN_WIDTH - leftPadding, SCREEN_HEIGHT)));
	}

	m_assetEditorButton.Render(RenderInfo({ 0, 0 }, { 120, 20 }));

	//NOTE: popup manager MUST be the last editor GUI to be rendered
	m_popupManager.RenderPopups();

	return true;
}