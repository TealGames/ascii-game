#pragma once
#include "raylib.h"
#include <unordered_map>
#include <map>
#include <string>
#include <optional>
#include "Direction.hpp"
#include "Vec2.hpp"
#include "Point2DInt.hpp"

enum class KeyState
{
	Neutral,
	Cooldown,
	Pressed,
	Down,
	Up,
};
std::string ToString(const KeyState& state);

class InputState
{
private:
	KeyState m_keyState;
	float m_cooldownTime;
	float m_currentCooldownTime;

public:
	/// <summary>
	/// This is used on the cooldowmn time var to signify no cooldown
	/// </summary>
	static constexpr float NO_COOLDOWN_TIME = -1;

private:
	InputState(const KeyState& startState, const float& cooldownTime);
public:
	InputState();
	InputState(const float& cooldown);
	
	bool HasCooldown() const;
	bool InCooldown() const;
	void ResetDefault();
	const float& GetCooldownTime() const;
	const float& GetCurrentCooldownTime() const;

	void SetCooldownDelta(const float& delta);
	
	const KeyState& GetState() const;
	void SetState(const KeyState& newState);
	bool IsState(const KeyState& state) const;
	
	bool IsPressed() const;
	bool IsDown() const;
	bool IsUp() const;
};

//TODO: predefined data like compounds should be mutated and set up to work with file loading
//rather than force user to add all compounds themselves (should leave option, but mainly all should be 
//loaded from memory, along with other input settings)
using CompoundDirectionCollection = std::unordered_map<Direction, KeyboardKey>;
class CompoundInput
{
private:
	CompoundDirectionCollection m_dirKeys;
public:

private:
public:
	CompoundInput(const CompoundDirectionCollection& keys);

	std::optional<KeyboardKey> TryGetDirection(const Direction& dir) const;
	const CompoundDirectionCollection& GetEntries() const;
	std::size_t GetEntriesCount() const;
};


using KeyStateCollection = std::unordered_map<KeyboardKey, InputState>;
using CompoundInputCollection = std::unordered_map<std::string, CompoundInput>;
class InputManager
{
private:
	KeyStateCollection m_keyStates;
	CompoundInputCollection m_compoundInput;

public:
	

private:
	bool IsKeyState(const KeyboardKey& key, const KeyState& state) const;

	KeyStateCollection::iterator TryGetKeyIteratorMutable(const KeyboardKey& key);
	KeyStateCollection::const_iterator TryGetKeyIterator(const KeyboardKey& key) const;

	CompoundInputCollection::iterator TryGetCompoundIteratorMutable(const std::string& name);
	CompoundInputCollection::const_iterator TryGetCompoundIterator(const std::string& name) const;

public:
	InputManager();
	InputManager(const std::map<KeyboardKey, float>& keyCooldownTime);
	InputManager(const float& allKeyCooldownTime);

	std::vector<KeyboardKey> GetAllKeys() const;

	const KeyState& GetKeyState(const KeyboardKey& key) const;
	bool IsKeyPressed(const KeyboardKey& key) const;
	bool IsKeyDown(const KeyboardKey& key) const;
	bool IsKeyUp(const KeyboardKey& key) const;

	bool TryAddCompoundInput(const std::string& name, const CompoundDirectionCollection& keys);
	std::optional<KeyboardKey> TryGetCompoundDirection(const std::string& name, const Direction& dir) const;
	std::optional<Utils::Point2DInt> TryGetCompoundInputDown(const std::string& name) const;
	std::vector<KeyState> GetCompoundKeyStates(const std::string& name) const;
	std::string GetCompoundToString(const std::string& name) const;

	void Update(const float& deltaTime);
};

