#pragma once
#include <unordered_map>
#include <map>
#include <string>
#include <optional>
#include <filesystem>
#include "Direction.hpp"
#include "raylib.h"
#include "Vec2.hpp"
#include "Point2DInt.hpp"
#include "CompoundInput.hpp"
#include "InputKey.hpp"
#include "InputProfile.hpp"
#include "ScreenPosition.hpp"
#include "InputProfileAsset.hpp"

//TODO: predefined data like compounds should be mutated and set up to work with file loading
//rather than force user to add all compounds themselves (should leave option, but mainly all should be 
//loaded from memory, along with other input settings)

namespace AssetManagement
{
	class AssetManager;
}

/// <summary>
/// The hierarchy/description for the Input system is as follows:
/// Input Manager-> manages Profiles, and updates input state for inputkeys
/// InputState-> current state of Key
/// InputKey-> holds key info as well as InputState for that key
/// InputAction-> holds multiple inputKeys and other info about an action
/// CompoundInput-> can hold multiple input actions and is most useful for directional inputs
/// </summary>
namespace Input
{
	class InputManager
	{
	private:
		static const std::filesystem::path INPUT_PROFILES_FOLDER;

		AssetManagement::AssetManager& m_assetManager;
		std::unordered_map<std::string, InputProfileAsset*> m_profiles;

		std::unordered_map<KeyboardKey, InputKey> m_keyboardStates;
		std::unordered_map<MouseButton, InputKey> m_mouseStates;
		std::unordered_map<GamepadButton, InputKey> m_gamepadStates;

		std::vector<int> m_capturedKeys;
		std::string m_charKeysPressed;

	public:
		static const std::string PROFILE_PREFIX;

	private:
		bool IsKeyDown(const DeviceType& device, const int& keyValue);
		bool IsKeyPressed(const DeviceType& device, const int& keyValue);
		bool IsKeyReleased(const DeviceType& device, const int& keyValue);

		void UpdateState(const DeviceType& device, const int& keyValue, 
			InputState& inputState, const float& deltaTime);

	public:
		InputManager(AssetManagement::AssetManager& assetManager);

		void SetInputCooldown(const std::map<KeyboardKey, float>& keyCooldownTime);
		void SetInputCooldown(const float& allKeyCooldownTime);
		void Update(const float& deltaTime);

		//void AddProfile(const std::string& name, const std::filesystem::path& profilePath);
		const InputProfile* TryGetProfile(const std::string& name) const;

		bool IsKeyState(const KeyboardKey& key, const KeyState& state) const;

		std::vector<KeyboardKey> GetAllKeyboardKeys();
		std::vector<MouseButton> GetAllMouseButtons();
		std::vector<GamepadButton> GetAllGamepadButtons();

		const KeyState& GetKeyState(const KeyboardKey& key) const;
		bool IsKeyPressed(const KeyboardKey& key) const;
		bool IsKeyDown(const KeyboardKey& key) const;
		bool IsKeyReleased(const KeyboardKey& key) const;

		std::vector<const InputKey*> GetAllKeysWithState(const KeyState& state) const;
		std::vector<std::string> GetAllKeysWithStateAsString(const KeyState& state) const;

		std::string GetCharsPressedSinceLastFrame() const;
		/// <summary>
		/// Gets the current mouses's position. Note: pos is screen coordinate
		/// but uses floats for more precision
		/// </summary>
		/// <returns></returns>
		Vec2 GetMousePosition() const;

		const InputKey* GetInputKey(const KeyboardKey& key) const;
		const InputKey* GetInputKey(const MouseButton& button) const;
		const InputKey* GetInputKey(const GamepadButton& button) const;
	};
}

