#pragma once
#include <unordered_map>
#include <map>
#include <string>
#include <optional>
#include <filesystem>
#include "Core/Primitives/Direction.hpp"
#include "Core/Input/CompoundInput.hpp"
#include "Core/Input/InputKey.hpp"
#include "Core/Primitives/ScreenPosition.hpp"
#include "Core/Asset/InputProfileAsset.hpp"

//TODO: predefined data like compounds should be mutated and set up to work with file loading
//rather than force user to add all compounds themselves (should leave option, but mainly all should be 
//loaded from memory, along with other input settings)

namespace Engine::Assets { class AssetManager; }
namespace Engine::Core { class WindowManager; }

/// <summary>
/// The hierarchy/description for the Input system is as follows:
/// Input Manager-> manages Profiles, and updates input state for inputkeys
/// InputState-> current state of Key
/// InputKey-> holds key info as well as InputState for that key
/// InputAction-> holds multiple inputKeys and other info about an action
/// CompoundInput-> can hold multiple input actions and is most useful for directional inputs
/// </summary>
namespace Engine::Input
{
	using AssetManager = Engine::Assets::AssetManager;
	class InputManager
	{
	private:
		static const std::filesystem::path INPUT_PROFILES_FOLDER;

		AssetManager& m_assetManager;
		std::unordered_map<std::string, InputProfileAsset*> m_profiles;

		mutable std::unordered_map<KeyCode, InputKeyState> m_keyStates;
		ScreenPosition m_mousePos;
		ScreenPosition m_lastFrameMousePos;

		/// <summary>
		/// Stores all the keys that have been added this frame
		/// </summary>
		std::vector<KeyCode> m_frameKeyQueue;
		std::string m_charKeysPressed;

	public:
		static const std::string PROFILE_PREFIX;

	private:
		auto LazyAddKeyState(const KeyCode code) const;
		void ForceAddMissingKeys() const;
		void ThrowIfNullKeyCode(const KeyCode code) const;
		void UpdateState(InputState& inputState, const float& deltaTime);

	public:
		InputManager(AssetManager& assetManager, Core::WindowManager& windowManager);
		void Init();

		void SetInputCooldown(const std::map<KeyCode, float>& keyCooldownTime);
		void SetInputCooldown(const float& allKeyCooldownTime);
		void Update(const float& deltaTime);
		void UpdateEnd();

		//void AddProfile(const std::string& name, const std::filesystem::path& profilePath);
		const InputProfile* TryGetProfile(const std::string& name) const;

		bool IsKeyState(const KeyCode& key, const KeyState& state) const;

		KeyState GetKeyState(const KeyCode& key) const;
		bool IsKeyPressed(const KeyCode& key) const;
		bool IsKeyDown(const KeyCode& key) const;
		bool IsKeyReleased(const KeyCode& key) const;

		std::vector<const InputKeyState*> GetAllKeysWithState(const KeyState& state) const;
		std::vector<std::string> GetAllKeysWithStateAsString(const KeyState& state) const;

		std::string GetCharsPressedSinceLastFrame() const;
		/// <summary>
		/// Gets the current mouses's position. Note: pos is screen coordinate
		/// but uses floats for more precision.
		/// </summary>
		/// <returns></returns>
		ScreenPosition GetMousePosition() const;
		ScreenPosition GetMousePositionDelta() const;


		const InputKeyState* GetInputKey(const KeyCode& key) const;

		std::string ToStringAllStates() const;
	};
}

