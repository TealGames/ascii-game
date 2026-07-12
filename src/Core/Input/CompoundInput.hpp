#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <array>
#include <optional>
#include "Core/Input/InputAction.hpp"
#include "Core/Input/InputDirection.hpp"

namespace Engine::Input
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
		Vec3Int GetInputWithState(KeyState state) const;

		template<size_t N>
		Vec3Int GetInputWithState(const std::array<KeyState, N>& states) const
		{
			Vec3Int dir = {};
			bool dirKeyHasState = false;
			for (const auto& entry : m_dirKeys)
			{
				/*LogError(std::format("Compound: {} key: {} STATE: {} (down: {})", name, std::to_string(entry.second),
					ToString(GetKeyState(entry.second)), std::to_string(IsKeyDown(entry.second))));*/
				dirKeyHasState = false;
				for (const auto& state : states)
				{
					if (entry.second.IsState(state))
					{
						dirKeyHasState = true;
						break;
					}
				}
				if (!dirKeyHasState) continue;

				AddDirectionToVector(dir, entry.first);
			}
			//LogError(std::format("When retrieving compound: {} -> {}", name, dir.ToString()));

			return dir;
		}
		template<size_t N>
		Vec3 GetInputWithStateNormalized(const std::array<KeyState, N>& states) const
		{
			Vec3Int input = GetInputWithState<N>(states);
			return Vec3(input.m_X, input.m_Y, input.m_Z).GetNormalized();
		}
		/*std::vector<KeyState> GetCompoundKeyStates();*/

		bool HasDirection(const InputDirection& dir) const;
		void AddEntry(const InputDirection& dir, const InputAction& action);

		const std::string& GetName() const;

		std::string ToString() const;
	};
}

