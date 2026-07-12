#include "pch.hpp"
#include <numbers>
#include "Editor/EngineEditor.hpp"
#include "Core/Time/TimeKeeper.hpp"
#include "ECS/Systems/Types/World/PlayerSystem.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/Camera/CameraController.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "Core/UI/PopupUIManager.hpp"
#include "Core/Physics/PhysicsManager.hpp"
#include "StaticGlobals.hpp"
#include "Core/UI/UIInteractionManager.hpp"
#include "ECS/Systems/Types/World/CollisionBoxSystem.hpp"
#include "Editor/Entity/ColorPopupUI.hpp"
#include "Editor/EditorStyles.hpp"
#include "Core/UIElementTemplates.hpp"
#include "Editor/GizmoOverlay.hpp"
#include "ECS/Component/Types/UI/UIToggleComponent.hpp"
#include "ECS/Component/Types/UI/UITransformComponent.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"
#include "ECS/Component/Types/UI/UIButtonComponent.hpp"
#include "ECS/Component/Types/UI/UILayoutComponent.hpp"
#include "Math/PlatformMath.hpp"

namespace Engine::Editor
{
	static constexpr float TOP_BAR_HEIGHT = 0.03;
	static constexpr float ASSET_EDITOR_BUTTON_WIDTH = 0.2;
	static constexpr float TOGGLE_LAYOUT_WIDTH_PER_TOGGLE = 0.03;
	static constexpr float TOFFLE_LAYOUT_WIDTH_SPACING = 0.05;
	static const NormalizedVec2 MOUSE_POS_TEXT_SIZE = { 0.1, 0.05 };

	static constexpr Input::KeyCode PAUSE_TOGGLE_KEY = Input::KeyCode::P;
	static constexpr Input::KeyCode SELECT_KEY = Input::KeyCode::MouseLeft;
	static constexpr float HELD_TIME_FOR_OBJECT_MOVE = 0.2;
	static constexpr Vec3 CAMERA_MOVE_SPEED = { 0.2, 0.2, 0.2 };
	static constexpr float MOUSE_SENSITIVITY = 0.5;
	static constexpr Vec2 CAMERA_ROTATE_RADIANS_PER_POS = Vec2(1.0f / SCREEN_WIDTH, 1.0f / SCREEN_HEIGHT) * float(std::numbers::pi);

	EditModeInfo::EditModeInfo() : m_Selected(nullptr) {}

	EngineEditor::EngineEditor(Core::TimeKeeper& time, const Input::InputManager& input, Physics::PhysicsManager& physics, Assets::AssetManager& assetManager,
		Scenes::SceneManager& scene, Camera::CameraController& camera, MainUI::UIInteractionManager& selector, MainUI::UIHierarchy& guiTree,
		MainUI::PopupUIManager& popupManager, Physics::CollisionBoxSystem& collisionSystem, MainDebug::CommandController& commands, Debug::GizmoOverlay& gizmos) :
		m_editorRoot(nullptr),
		m_displayingGameView(true),
		m_timeKeeper(time), m_inputManager(input), m_sceneManager(scene), m_cameraController(camera),
		m_physicsManager(physics), m_guiSelector(selector), m_guiTree(guiTree), m_collisionBoxSystem(collisionSystem), 
		m_commandController(commands), m_commandConsole(m_commandController, m_inputManager), m_debugInfo(), m_gizmos(gizmos),
		m_popupManager(popupManager), m_entityEditor(m_inputManager, m_cameraController, m_guiTree, m_popupManager, assetManager),
		m_spriteEditor(m_guiTree, m_inputManager, assetManager), 
		m_overheadBarContainer(nullptr), m_toggleLayout(nullptr), m_pauseGameToggle(nullptr), m_editModeToggle(nullptr), m_mousePosText(nullptr),
		m_editModeInfo(), m_assetEditorButton(nullptr), m_inCameraFreemode(true), m_inputProfile(nullptr), m_freelookPitch(), m_freelookYaw()
	{

	}

	EngineEditor::~EngineEditor()
	{

	}

	void EngineEditor::InitConsoleCommands(Player::PlayerSystem& playerSystem)
	{
		m_commandController.AddCommand(new MainDebug::ActionCommand<std::string, float, float, float>("setpos", { "EntityName", "PosX", "PosY", "PosZ" },
			[this](const std::string& entityName, const float x, const float y, const float z) -> void {
				if (ECS::EntityData* entity = m_sceneManager.GetActiveSceneMutable()->TryGetEntityMutable(entityName, true))
				{
					entity->GetTransformMutable().GetLocalPosMutable() = WorldPosition3D(x, y, z);
				}
			}));

		m_commandController.AddCommand(new MainDebug::ActionCommand<std::string>("editor", { "EntityName" },
			[this](const std::string& entityName) -> void {
				if (ECS::EntityData* entity = m_sceneManager.GetActiveSceneMutable()->TryGetEntityMutable(entityName, true))
				{
					//Assert(false, std::format("Sending entity: {}", entity->m_Name));
					m_entityEditor.SetEntityUI(*entity);
					return;
				}
				m_commandConsole.LogOutputMessage(std::format("Entity with name: '{}' could not be found",
					entityName), UI::ConsoleUIOutputMessageType::Error);
			}));

		m_commandController.AddCommand(new MainDebug::ActionCommand<>("docs", std::vector<std::string>{},
			[this]() -> void {
				m_commandConsole.LogOutputMessages(
					m_commandController.GetCommandDocumentationAll(), UI::ConsoleUIOutputMessageType::Default);
			}));

		m_commandController.AddCommand(new MainDebug::ActionCommand<std::string>("debugmessage", std::vector<std::string>{"MessageFilter"},
			[](const std::string& messageFilter) -> void {
				DebugProperties::SetLogMessageFilter(messageFilter);
			}));

		m_commandController.AddCommand(new MainDebug::ActionCommand<std::string>("debugtype", std::vector<std::string>{"MessageType"},
			[this](const std::string& typeFilter) -> void {
				std::optional<LogType> maybeLogType = StringToLogType(typeFilter);
				if (maybeLogType == std::nullopt)
				{
					m_commandConsole.LogOutputMessage(std::format("Invalid LogType: {}",
						typeFilter), UI::ConsoleUIOutputMessageType::Error);
					return;
				}

				DebugProperties::SetLogTypeFilter(maybeLogType.value());
				/*LogError(std::format("The new log type: {} has log: {}", LogTypeToString(GetLogTypeFilter()),
					std::to_string(::Utils::HasFlagAny(GetLogTypeFilter(), LogType::Log))));*/
					//throw std::invalid_argument("POOP");
			}));

		m_commandController.AddCommand(new MainDebug::ActionCommand<>("debugreset", std::vector<std::string>{},
			[this]() -> void {
				DebugProperties::ResetLogFilters();
			}));

		m_commandController.AddCommand(new MainDebug::ActionCommand<float>("settimescale", std::vector<std::string>{"TimeScale"},
			[this](const float& scale) -> void {
				m_timeKeeper.SetTimeScale(scale);
			}));

		m_commandController.AddCommand(new MainDebug::ActionCommand<bool>("cheats", std::vector<std::string>{"CheapStatus"},
			[&playerSystem](const bool& enableCheats) -> void {
				playerSystem.SetCheatStatus(enableCheats);
			}));

		m_commandController.AddCommand(new MainDebug::ActionCommand<>("save", std::vector<std::string>{},
			[this]() -> void {
				m_sceneManager.SaveCurrentScene();
			}));
	}

	void EngineEditor::SelectEntityEditor(ECS::EntityData& entity)
	{
		m_entityEditor.SetEntityUI(*m_editModeInfo.m_Selected);
		m_popupManager.CloseAllPopups();
	}

	void EngineEditor::Init(Player::PlayerSystem& playerSystem)
	{
		m_inputProfile = m_inputManager.TryGetProfile(MAIN_INPUT_PROFILE_NAME);
		if (m_inputProfile == nullptr)
		{
			LogError(std::format("Tried to get input profile:'{}' in engine editor but it was not found.",
				MAIN_INPUT_PROFILE_NAME));
			return;
		}

		m_editorRoot = std::get<1>(m_guiTree.CreateAtRoot(MainUI::DEFAULT_LAYER, "EditorRoot"));
		ECS::EntityData& editorRootEntity = m_editorRoot->GetEntityMutable();
		m_entityEditor.Init(editorRootEntity);

		auto [mousePosTextEntity, mousePosTextTransform] = editorRootEntity.CreateChildUI("MousePosText");
		m_mousePosText = &(mousePosTextEntity->AddComponent(MainUI::UITextComponent("", Styles::GetTextStyleFactorSize(MainUI::TextAlignment::Center))));
		mousePosTextTransform->SetLocalSize(MOUSE_POS_TEXT_SIZE);

		//----------------------------------------------------------------
		// OVERHEAD BAR CREATION + TOGGLE LAYOUT + ASSET EDITOR BUTTON
		//---------------------------------------------------------------
		auto [overheadBarEntity, overheadBarTransform] = editorRootEntity.CreateChildUI("OverheadBar");
		m_overheadBarContainer = &(overheadBarEntity->AddComponent(MainUI::UIPanelComponent(Styles::EDITOR_BACKGROUND_COLOR)));
		overheadBarTransform->SetLocalBoundsTLBR(MainUI::UI_RECT_TOP_LEFT, { 1, 1 - TOP_BAR_HEIGHT });

		auto [toggleLayoutEntity, toggleLayoutTransform] = m_overheadBarContainer->GetEntityMutable().CreateChildUI("ToggleLayout");
		m_toggleLayout = &(toggleLayoutEntity->AddComponent(MainUI::UILayoutComponent(MainUI::LayoutType::Horizontal,
			MainUI::SizingType::ShrinkOnly, { TOFFLE_LAYOUT_WIDTH_SPACING, 0 })));

		auto [assetEditorButtonEntity, assetEditorButtonTransform] = m_overheadBarContainer->GetEntityMutable().CreateChildUI("AssetEditorButton");
		m_assetEditorButton = &(assetEditorButtonEntity->AddComponent(MainUI::UIButtonComponent(Styles::GetButtonStyle(MainUI::TextAlignment::Center))));
		m_assetEditorButton->SetText("AssetEditors");
		assetEditorButtonTransform->SetLocalBoundsTLBR(MainUI::UI_RECT_TOP_LEFT, { ASSET_EDITOR_BUTTON_WIDTH, 0 });
		m_assetEditorButton->AddClickAction([this](const MainUI::UIButtonComponent& data)-> void
			{
				m_displayingGameView = !m_displayingGameView;
				if (m_displayingGameView)
				{
					m_assetEditorButton->SetText("AssetEditors");
					m_entityEditor.CloseCurrentEntityGUI();
				}
				else m_assetEditorButton->SetText("GameView");
			});

		//----------------------------------------------------------------
		// PAUSE AND EDIT MODE TOGGLE CREATION
		//----------------------------------------------------------------
		ECS::EntityData* pauseGameToggleEntity = nullptr;
		MainUI::UITransformComponent* pauseGameToggleTransform = nullptr;
		std::tie(pauseGameToggleEntity, pauseGameToggleTransform, m_pauseGameToggle) = MainUI::Templates::CreateCheckboxTemplate(*toggleLayoutEntity, "PauseGameToggle");
		pauseGameToggleTransform->SetFixed(false, true);
		m_pauseGameToggle->m_OnValueSet.AddListener([this](const bool isChecked) -> void
			{
				//LogError("Pause game toggle is set with val:{}", isChecked);
				if (isChecked) m_timeKeeper.StopTimeScale();
				else m_timeKeeper.ResetTimeScale();
			});

		ECS::EntityData* editModeToggleEntity = nullptr;
		MainUI::UITransformComponent* editModeToggleTransform = nullptr;
		std::tie(editModeToggleEntity, editModeToggleTransform, m_editModeToggle) = MainUI::Templates::CreateCheckboxTemplate(*toggleLayoutEntity, "EditModeToggle");
		editModeToggleTransform->SetFixed(false, true);
		m_editModeToggle->m_OnValueSet.AddListener([this](const bool isChecked)-> void
			{
				m_mousePosText->GetEntityMutable().TrySetEntityActive(isChecked);
				if (isChecked) m_gizmos.EnableAllGizmos();
				else m_gizmos.DisableAllGizmos();
			});

		const float layoutWidth = TOGGLE_LAYOUT_WIDTH_PER_TOGGLE * toggleLayoutEntity->GetChildCount();
		const float layoutStartX = (1 - layoutWidth) / 2;
		toggleLayoutTransform->SetLocalBoundsTLBR({ layoutStartX, 1 }, { layoutStartX + layoutWidth, 0 });


		//m_assetEditorButton.SetSettings(buttonSettings);
		DebugProperties::OnMessageLogged.AddListener([this](const LogType& logType, const std::string& message,
			const bool eventFlag)-> void
			{
				if (!eventFlag || logType != LogType::Error) return;

				//To prevent paused going unpaused here we need it to be not paused state
				if (!m_pauseGameToggle->IsToggled())
					m_pauseGameToggle->ToggleValue();
			});

		//LogWarning(std::format("Popup addr:{}", ::Utils::ToStringPointerAddress(&m_popupManager)));

		m_commandConsole.CreateUI(m_guiTree);
		InitConsoleCommands(playerSystem);

		m_debugInfo.CreateUI(m_guiTree);
		//Note: the init order matters because it creates the order that the objects are added to the selector
		m_popupManager.AddPopup(new UI::ColorPopupUI(m_inputManager));

		m_freelookYaw = ::Math::RAD_180;
		m_freelookPitch = 0;
		const Vec3 initialCameraRotation = Vec3(m_freelookPitch, m_freelookYaw, 0);
		const TransformComponent cameraTransform = TransformComponent(Vec3(0, 0.1, 0.5), Vec3::One(), Math::ToQuaternion(initialCameraRotation));
		ECS::EntityData& mainCameraEntity = m_sceneManager.m_GlobalEntityManager.CreateGlobalEntity("EditorCamera", cameraTransform);
		Camera::CameraComponent& cameraData = mainCameraEntity.AddComponent(Camera::CameraComponent{ Camera::CameraSettings{SCREEN_ASPECT_RATIO, 10, nullptr} });
		m_cameraController.TryRegisterCamera(cameraData);
	}

	void EngineEditor::Update(const float unscaledDeltaTime, const float scaledDeltaTime, const float timeStep)
	{
		//LogError(std::format("Tree is:{}", m_guiTree.ToStringTree()));
		m_commandConsole.Update(scaledDeltaTime);

		if (!IsInGameView())
		{
			m_spriteEditor.Update(unscaledDeltaTime);
			return;
		}

		Scenes::Scene* activeScene = m_sceneManager.GetActiveSceneMutable();
		if (!Assert(activeScene != nullptr, "Tried to update the engine editor but "
			"there are no active scenes right now", activeScene->GetName()))
			return;

		Camera::CameraComponent& mainCamera = m_cameraController.GetActiveCameraMutable();

		//Assert(false, std::format("Entity editor update"));
		//m_commandConsole.Update();
		m_debugInfo.Update(unscaledDeltaTime, timeStep, *activeScene, m_inputManager, mainCamera);

		//m_pauseGameToggle.Update();
		if (m_inputManager.IsKeyPressed(PAUSE_TOGGLE_KEY))
		{
			//LogError("Toggle pause");
			m_pauseGameToggle->ToggleValue();
		}

		//m_editModeToggle.Update();
		//LogError(std::format("Is toggled:{} selected:{}", std::to_string(m_editModeToggle.IsToggled()), std::to_string(m_editModeInfo.m_Selected != nullptr)));

		const Vec2 mousePos = m_inputManager.GetMousePosition();
		const Vec2 mouseDelta = m_inputManager.GetMousePositionDelta();
		//LogWarning(std::format("MOuse dleta:{}", mouseDelta.ToString()));
		Math::Ray3D worldClickedRay = mainCamera.ScreenToWorldPosition(ScreenPosition(mousePos.m_X, mousePos.m_Y));
		if (m_inputManager.GetInputKey(SELECT_KEY)->GetState().IsPressed())
		{
			auto entitiesWithinPos = m_collisionBoxSystem.FindBodiesContainingPos(*activeScene, worldClickedRay.m_Origin.GetXY());
			if (!entitiesWithinPos.empty())
			{
				m_editModeInfo.m_Selected = &(entitiesWithinPos[0]->GetEntityMutable());
				SelectEntityEditor(*m_editModeInfo.m_Selected);
				//LogError(std::format("Tree is:{}", m_guiTree.ToStringTree()));
			}
		}
		else if (m_editModeToggle->IsToggled())
		{
			//If we are in edit mode holding the down button (and not selected selectable this frame-> meaning click might correspond to selectable click not edit mode click) 
			// we can move the selected entity to that pos
			//Note: we only change the xy and keep z the same since that cannot be resolved just from a screen click
			if (m_inputManager.GetInputKey(SELECT_KEY)->GetState().IsDownForTime(HELD_TIME_FOR_OBJECT_MOVE) &&
				!m_guiSelector.SelectedSelectableThisFrame() && m_editModeInfo.m_Selected != nullptr)
			{
				/*Assert(false, std::format("Is down for:{} needed:{}", std::to_string(m_inputManager.GetInputKey(MOUSE_BUTTON_LEFT)->GetState().GetCurrentDownTime()),
				std::to_string(HELD_TIME_FOR_OBJECT_MOVE)));*/
				m_editModeInfo.m_Selected->GetTransformMutable().GetLocalPosMutable().SetXY(worldClickedRay.m_Origin.GetXY());
			}

			const Vec2 mousePosNorm = Vec2(mousePos.m_X / SCREEN_WIDTH, (SCREEN_HEIGHT - mousePos.m_Y) / SCREEN_HEIGHT);
			m_mousePosText->SetText(mousePos.ToString(2));

			MainUI::UITransformComponent& mousePosTextTransform = *(m_mousePosText->GetEntityMutable().TryGetComponentMutable<MainUI::UITransformComponent>());
			const Vec2 textSize = mousePosTextTransform.GetLocalSize().AsVec2();
			mousePosTextTransform.SetLocalTopLeftPos({ mousePosNorm.m_X - (textSize.m_X / 2), mousePosNorm.m_Y + textSize.m_Y });
		}
		m_entityEditor.Update();

		if (m_inCameraFreemode)
		{
			if (mouseDelta != Vec2::Zero())
			{
				constexpr float pi = std::numbers::pi_v<float>;
				const Vec2 rotationInput = mouseDelta * CAMERA_ROTATE_RADIANS_PER_POS * MOUSE_SENSITIVITY;

				m_freelookPitch += rotationInput.m_Y;
				m_freelookPitch = std::clamp(m_freelookPitch, -pi / 2 + 0.01f, pi / 2 - 0.01f);
				m_freelookYaw += rotationInput.m_X;
				/*LogWarning(std::format("Rtoation input: {} pitch:{} yaw:{}", rotationInput.ToString(), m_freelookPitch, m_freelookYaw));*/

				//Quat yawQuat = Quat::FromAxisAngle(ENGINE_UP_DIR, m_freelookYaw);
				//Quat pitchQuat = Quat::FromAxisAngle(ENGINE_RIGHT_DIR, m_freelookPitch);

				Math::Quat yawQuat = Math::ToQuaternion(ENGINE_UP_DIR, m_freelookYaw);
				Vec3 rotatedRight = yawQuat.ApplyRotationToDir(ENGINE_RIGHT_DIR);
				Math::Quat pitchQuat = Math::ToQuaternion(rotatedRight, m_freelookPitch);

				// Step 4: combine them — yaw first, then pitch
				mainCamera.GetTransformMutable().GetLocalRotationMutable() = pitchQuat * yawQuat;
			}

			const Input::CompoundInput* moveCompound = m_inputProfile->TryGetCompoundInputAction(MAIN_INPUT_PROFILE_MOVE_ACTION);
			if (moveCompound == nullptr)
			{
				LogError(std::format("Tried to get move compound input:'{}' in engine editor for profile:{} but failed",
					MAIN_INPUT_PROFILE_MOVE_ACTION, MAIN_INPUT_PROFILE_NAME));
				return;
			}
			const Vec3Int pressedDir = moveCompound->GetInputWithState<2>({ Input::KeyState::Down, Input::KeyState::Pressed });
			/*LogWarning(std::format("DONW DIR: {} non normal:{} compoound:{}", downDirNormalized.ToString(),
				moveCompound->GetCompoundInputDown().ToString(), moveCompound->ToString()));*/
				//LogWarning(std::format("pressed dir: {}", pressedDir.ToString()));
			if (pressedDir != Vec3Int::Zero())
			{
				//LogWarning(std::format("World forward of camera: {}s", mainCamera.CalculateWorldForward().ToString()));
				//const Vec3 rotatedDir= mainCamera.GetTransformMutable().GetGlobalRotation().ApplyRotationToDir(ENGINE_FORWARD_DIR);
				Vec3 facingDir = mainCamera.GetTransform().CalculateWorldForward();
				mainCamera.GetTransformMutable().GetLocalPosMutable() += mainCamera.GetTransform().GetLocalRotation().ApplyRotationToDir(pressedDir.AsFloat())
					//* mainCamera.CalculateWorldForward() 
					* CAMERA_MOVE_SPEED * unscaledDeltaTime;

				/*LogWarning(std::format("Camera pressed dir:{} facedir:{} newRotnewPos:{}", pressedDir.ToString(), facingDir.ToString(),
					(pressedDir.AsFloat() * facingDir * CAMERA_MOVE_SPEED * unscaledDeltaTime).ToString()));*/
			}
			//LogWarning(std::format("Camera transform:{}", mainCamera.GetTransformMutable().ToString()));
			//mainCamera.GetTransformMutable().GetLocalRotationMutable() *= Vec3(0, 0.13 * unscaledDeltaTime, 0);
		}

		//LogWarning(std::format("pause toggle:{} edit toggle:{}", m_pauseGameToggle->IsToggled(), m_editModeToggle->IsToggled()));
	}

	bool EngineEditor::IsInGameView() const
	{
		return m_displayingGameView;
	}
}
