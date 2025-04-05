#include "pch.hpp"
#include "EngineEditor.hpp"
#include "TimeKeeper.hpp"
#include "PlayerSystem.hpp"

EngineEditor::EngineEditor(TimeKeeper& time, const Input::InputManager& input, Physics::PhysicsManager& physics,
	SceneManagement::SceneManager& scene, GUISelectorManager& selector)
	: m_timeKeeper(time), m_inputManager(input), m_sceneManager(scene), m_commandConsole(m_inputManager, selector), m_debugInfo(),
	m_entityEditor(m_inputManager, m_sceneManager, physics, selector), m_pauseGameToggle(selector, false, GUISettings())
{
	m_pauseGameToggle.SetSettings(GUISettings({20, 20}, EntityEditorGUI::EDITOR_SECONDARY_COLOR,
		TextGUISettings(EntityEditorGUI::EDITOR_TEXT_COLOR, FontData(0, GetGlobalFont()), EntityEditorGUI::EDITOR_CHAR_SPACING.m_X, TextAlignment::Center, 0.8)));
	m_pauseGameToggle.SetValueSetAction([this](const bool isChecked) -> void
		{ 
			if (isChecked) m_timeKeeper.StopTimeScale();
			else m_timeKeeper.ResetTimeScale();
			//Assert(false, "CLICKED");
		});
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

void EngineEditor::Update(const float& deltaTime, const float& timeStep, 
	const Scene& activeScene, CameraData& mainCamera)
{
	//Assert(false, std::format("Entity editor update"));
	m_commandConsole.Update();
	m_entityEditor.Update(mainCamera);
	m_debugInfo.Update(deltaTime, timeStep, activeScene, m_inputManager, mainCamera);

	m_pauseGameToggle.Update();
}

bool EngineEditor::TryRender()
{
	m_commandConsole.TryRender();
	m_entityEditor.TryRender();
	m_debugInfo.TryRender();

	m_pauseGameToggle.Render(RenderInfo({0,0}, {20, 20}));
	return true;
}