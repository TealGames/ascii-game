#include "pch.hpp"
#include "Core/Input/InputManager.hpp"
#include "Utils/Debug.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Window/WindowManager.hpp"

//If the mouse pos delta magnitude is greater than this value, the mouse pos delta will be ignored
static constexpr int SINGLE_FRAME_MAX_MOUS_DELTA = 500;
namespace Input
{
	const std::filesystem::path InputManager::INPUT_PROFILES_FOLDER = "input";
	//const std::string InputManager::PROFILE_PREFIX = "profile_";

	auto InputManager::LazyAddKeyState(const KeyCode code) const
	{
		return m_keyStates.emplace(code, InputKeyState(code, InputState()));
	}

	InputManager::InputManager(AssetManagement::AssetManager& assetManager, Core::WindowManager& windowManager)
		: m_assetManager(assetManager), m_keyStates(), m_profiles{}, m_mousePos(INVALID_SCREEN_POS), m_lastFrameMousePos(INVALID_SCREEN_POS)
	{
		windowManager.m_OnInput.AddListener([this](Core::Window*, const Core::WindowInputEventInfo event) -> void
			{
				if (event.m_EventType == Core::WindowInputEventType::ButtonPress)
				{
					auto it = m_keyStates.find(event.m_KeyUpdated);
					if (it == m_keyStates.end()) it = LazyAddKeyState(event.m_KeyUpdated).first;

					it->second.GetStateMutable().SetState(event.m_KeyState);
					m_frameKeyQueue.emplace_back(event.m_KeyUpdated);
				}
				else if (event.m_EventType == Core::WindowInputEventType::MouseMove)
				{
					m_mousePos = event.m_NewCursorPos;
					const ScreenPosition thisFrameDelta = m_mousePos - m_lastFrameMousePos;
					if (std::abs(thisFrameDelta.m_X) > SINGLE_FRAME_MAX_MOUS_DELTA || std::abs(thisFrameDelta.m_Y) > SINGLE_FRAME_MAX_MOUS_DELTA)
					{
						m_lastFrameMousePos = m_mousePos;
					}
					//LogWarning(std::format("MOVED delta:{} pos:{}", m_lastFrameMousePos.ToString(), m_mousePos.ToString()));
				}
				else
				{
					LogError(std::format("Window input event occured but it has no actions defined in input manager"));
				}
				//LogError(std::format("key:{} state:{} all:{}", ToString(event.m_KeyUpdated), ToString(event.m_KeyState), ToStringAllStates()));
			});
	}

	void InputManager::Init()
	{
		//for (const auto& key : GetAllKeyboardKeys())
		//{
		//	m_keyStates.emplace(key, InputKeyState(key, InputState()));
		//}

		//if (allInputProfilePath.empty()) return;
		//LogWarning(std::format("Does input path exist:{}", std::to_string(m_assetManager.IsValidAssetPath(INPUT_PROFILES_FOLDER))));
		auto profiles = m_assetManager.GetAssetsOfTypeMutable<InputProfileAsset>(INPUT_PROFILES_FOLDER);
		if (!Assert(profiles.size() > 0, std::format("Tried to load all input profiles in input manager "
			"but could not find any input profile at path: '{}'", INPUT_PROFILES_FOLDER.string())))
			return;

		for (auto& profile : profiles)
		{
			if (profile == nullptr) continue;
			m_profiles.emplace(profile->GetName(), profile);
		}
		//LogError("Finished input manager");

		//std::string fileName = "";
		//try
		//{
		//	for (const auto& file : std::filesystem::directory_iterator(allInputProfilePath))
		//	{
		//		fileName = file.path().stem().string();
		//		if (!file.is_regular_file() || fileName.size() < PROFILE_PREFIX.size()) continue;
		//		if (fileName.substr(0, PROFILE_PREFIX.size()) != PROFILE_PREFIX) continue;

		//		//NOTE: we use the segment of the profile file after prefix for profile name
		//		AddProfile(fileName.substr(PROFILE_PREFIX.size()), file.path());
		//	}
		//}
		//catch (const std::exception& e)
		//{
		//	LogError(std::format("Tried to add all profiles at path: {} "
		//		"but ran into error: {}", allInputProfilePath.string(), e.what()));
		//}
	}
	void InputManager::ForceAddMissingKeys() const
	{
		for (const auto& keyboardKey : GetAllKeyboardKeys())
		{
			if (m_keyStates.find(keyboardKey) == m_keyStates.end())
				LazyAddKeyState(keyboardKey);
		}
		for (const auto& mouseButton : GetAllMouseButtons())
		{
			if (m_keyStates.find(mouseButton) == m_keyStates.end())
				LazyAddKeyState(mouseButton);
		}
		for (const auto& gamepadButton : GetAllGamepadButtons())
		{
			if (m_keyStates.find(gamepadButton) == m_keyStates.end())
				LazyAddKeyState(gamepadButton);
		}
	}
	void InputManager::ThrowIfNullKeyCode(const KeyCode code) const
	{
		if (code == KeyCode::Null)
		{
			LogError(std::format("Quering null key code is not supported"));
			throw std::invalid_argument("Invalid keycode");
		}
	}

	void InputManager::SetInputCooldown(const float& allKeyCooldownTime)
	{
		for (auto& key : m_keyStates)
		{
			key.second.GetStateMutable().SetCooldownTime(allKeyCooldownTime);
		}
	}
	void InputManager::SetInputCooldown(const std::map<KeyCode, float>& keyCooldownTime)
	{
		if (keyCooldownTime.empty()) return;

		auto it = m_keyStates.end();
		for (const auto& cooldownTime : keyCooldownTime)
		{
			it = m_keyStates.find(cooldownTime.first);
			if (it == m_keyStates.end()) continue;

			it->second.GetStateMutable().SetCooldownTime(cooldownTime.second);
		}
	}

	void InputManager::UpdateState(InputState& inputState, const float& deltaTime)
	{
		if (inputState.IsState(KeyState::Neutral))
			return;

		//First we update any deltas to cooldown
		if (inputState.InCooldown())
		{
			inputState.SetCooldownDelta(deltaTime);
		}

		//We then check again (in case we might have left cooldown after delta finished cooldown)
		if (!inputState.InCooldown())
		{
			if (inputState.GetKeyState() == KeyState::Pressed) {}
			else if (inputState.GetKeyState()== KeyState::Down)
			{
				//Only if the state is already down do we apply the delta time since if we just set it now
				//the held time might be off
				inputState.SetDownTimeDelta(deltaTime);
			}
			//If we are not pressing anything, but last frame we released and we have cooldown
			//we only set the state for cooldown so it gets updated next frame
			else if(inputState.IsReleased() && inputState.HasCooldown())
			{
				inputState.SetState(KeyState::Cooldown);
			}
		}
	}

	void InputManager::Update(const float& deltaTime)
	{
		//m_frameMousePosDelta = {};

		for (auto& inputState : m_keyStates)
		{
			UpdateState(inputState.second.GetStateMutable(), deltaTime);
		}
		//LogWarning(std::format("Keys pressed:{}", Utils::ToStringIterable(GetAllKeysWithStateAsString(KeyState::Pressed))));
		m_charKeysPressed.clear();
		
		char keyChar = 0;
		const bool shiftPressed = InputManager::IsKeyPressed(KeyCode::ShiftLeft) ||
								  InputManager::IsKeyPressed(KeyCode::ShiftRight);
		const bool capsLocked = InputManager::IsKeyPressed(KeyCode::CapsLock);

		for (size_t i=0; i<m_frameKeyQueue.size(); i++)
		{
			keyChar = GetKeyCodeAsChar(m_frameKeyQueue[i], shiftPressed, capsLocked);
			if (keyChar != 0)
				m_charKeysPressed += keyChar;
		}
		m_frameKeyQueue.clear();

		/*if (IsKeyPressed(KeyboardKey::KEY_TAB)) Assert(false, std::format("PRESS TAB"));
		LogWarning(std::format("Key state:{}", ToString(GetKeyState(KeyboardKey::KEY_TAB))));*/
	}
	void InputManager::UpdateEnd()
	{
		m_lastFrameMousePos = m_mousePos;
	}

	/*void InputManager::AddProfile(const std::string& name, const std::filesystem::path& profilePath)
	{
		m_profiles.emplace(name, InputProfile(*this, name, profilePath));
	}*/

	const InputProfile* InputManager::TryGetProfile(const std::string& name) const
	{
		auto it = m_profiles.find(name);
		if (it == m_profiles.end()) return nullptr;

		return &(it->second->GetProfile());
	}

	/*CompoundInputCollection::iterator TryGetCompoundIteratorMutable(const std::string& name)
	{
		return CompoundInput.find(name);
	}
	CompoundInputCollection::const_iterator TryGetCompoundIterator(const std::string& name)
	{
		return CompoundInput.find(name);
	}*/

	bool InputManager::IsKeyState(const KeyCode& key, const KeyState& state) const
	{
		ThrowIfNullKeyCode(key);
		auto it = m_keyStates.find(key);

		//If we have not registered a key -> it means it has no event yet
		//so here we can get away with no lazy instantiation and checking for neutral state
		if (it == m_keyStates.end())
			return state == KeyState::Neutral;

		return it->second.GetState().IsState(state);
	}
	KeyState InputManager::GetKeyState(const KeyCode& key) const
	{
		ThrowIfNullKeyCode(key);

		auto it = m_keyStates.find(key);
		if (it == m_keyStates.end())
		{
			it = LazyAddKeyState(key).first;
		}
			
		return it->second.GetState().GetKeyState();
	}
	bool InputManager::IsKeyPressed(const KeyCode& key) const
	{
		ThrowIfNullKeyCode(key);
		return IsKeyState(key, KeyState::Pressed);
	}
	bool InputManager::IsKeyDown(const KeyCode& key) const
	{
		ThrowIfNullKeyCode(key);
		return IsKeyState(key, KeyState::Down);
	}
	bool InputManager::IsKeyReleased(const KeyCode& key) const
	{
		ThrowIfNullKeyCode(key);
		return IsKeyState(key, KeyState::Released);
	}

	std::vector<const InputKeyState*> InputManager::GetAllKeysWithState(const KeyState& state) const
	{
		//Only if state is neutral (the default state for keys that are not pressed)
		//do we then have to force all missing keys to be added
		if (state == KeyState::Neutral)
			ForceAddMissingKeys();

		//TODO: what should happen considering we have lazy instantion for keys? should those be ignored?
		//or should we force add all if the state is only neutral?
		std::vector<const InputKeyState*> keys = {};
		for (const auto& key : m_keyStates)
		{
			if (key.second.GetState().IsState(state)) 
				keys.emplace_back(&key.second);
		}

		return keys;
	}
	std::vector<std::string> InputManager::GetAllKeysWithStateAsString(const KeyState& state) const
	{
		if (state == KeyState::Neutral)
			ForceAddMissingKeys();

		std::vector<std::string> keys = {};
		for (const auto& key : m_keyStates)
		{
			if (key.second.GetState().IsState(state))
				keys.emplace_back(key.second.ToString(false, false));
		}

		return keys;
	}

	std::string InputManager::GetCharsPressedSinceLastFrame() const
	{
		return m_charKeysPressed;
	}

	ScreenPosition InputManager::GetMousePosition() const
	{
		return m_mousePos;
	}
	ScreenPosition InputManager::GetMousePositionDelta() const
	{
		if (m_lastFrameMousePos == INVALID_SCREEN_POS)
			return ScreenPosition::Zero();

		return m_mousePos - m_lastFrameMousePos;
	}

	const InputKeyState* InputManager::GetInputKey(const KeyCode& key) const
	{
		ThrowIfNullKeyCode(key);

		auto it = m_keyStates.find(key);
		if (it == m_keyStates.end())
		{
			it = LazyAddKeyState(key).first;
		}
		
		return &(it->second);
	}

	std::string InputManager::ToStringAllStates() const
	{
		std::vector<std::string> statesStr = {};
		for (const auto& state : m_keyStates)
		{
			statesStr.push_back(state.second.ToString());
		}
		return Utils::ToStringIterable(statesStr);
	}

	/*bool TryAddCompoundInput(const std::string& name, const CompoundDirectionCollection& keys)
	{
		if (!Assert(TryGetCompoundIterator(name) == CompoundInput.end(),
			std::format("InputManager: Tried to add compound input with name: '{}' "
				"but that compound input binding already exists", name)))
			return false;

		CompoundInput.emplace(name, keys);
		return true;
	}*/
}