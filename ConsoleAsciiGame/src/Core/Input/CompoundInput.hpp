#pragma once
#include "raylib.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <optional>
#include "InputAction.hpp"
#include "InputDirection.hpp"
#include "Vec2Int.hpp"

namespace Input
{
	using CompoundDirectionCollection = std::unordered_map<InputDirection, InputAction>;
	class CompoundInput
	{
	private:
		std::string m_name;
		CompoundDirectionCollection m_dirKeys;

	public:

	private:
	public:
		CompoundInput();
		CompoundInput(const std::string& name);
		CompoundInput(const std::string& name, const CompoundDirectionCollection& keys);

		const CompoundDirectionCollection& GetEntries() const;
		std::size_t GetEntriesCount() const;

		const InputAction* TryGetDirectionAction(const InputDirection& dir) const;
		Vec2Int GetCompoundInputDown() const;
		/*std::vector<KeyState> GetCompoundKeyStates();*/

		bool HasDirection(const InputDirection& dir) const;
		void AddEntry(const InputDirection& dir, const InputAction& action);

		const std::string& GetName() const;

		std::string ToString() const;
	};
}

