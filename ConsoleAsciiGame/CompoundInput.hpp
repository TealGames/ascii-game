#pragma once
#include "raylib.h"
#include "Direction.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <optional>
#include "InputAction.hpp"

namespace Input
{
	using CompoundDirectionCollection = std::unordered_map<Direction, InputAction>;
	class CompoundInput
	{
	private:
		std::string m_name;
		CompoundDirectionCollection m_dirKeys;

	public:

	private:
	public:
		CompoundInput(const std::string& name);
		CompoundInput(const std::string& name, const CompoundDirectionCollection& keys);

		const CompoundDirectionCollection& GetEntries() const;
		std::size_t GetEntriesCount() const;

		const InputAction* TryGetDirectionAction(const Direction& dir) const;
		Utils::Point2DInt GetCompoundInputDown() const;
		/*std::vector<KeyState> GetCompoundKeyStates();*/

		bool HasDirection(const Direction& dir) const;
		void AddEntry(const Direction& dir, const InputAction& action);

		const std::string& GetName() const;

		std::string ToString() const;
	};
}

