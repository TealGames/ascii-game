#pragma once
#include <unordered_map>
#include <filesystem>
#include <string>
#include "InputAction.hpp"
#include "CompoundInput.hpp"

namespace Input
{
	class InputManager;

	using InputActionCollection = std::unordered_map<std::string, InputAction>;
	using CompoundInputCollection = std::unordered_map<std::string, CompoundInput>;
	class InputProfile
	{
	private:
		std::string m_name;
		InputActionCollection m_actions;
		CompoundInputCollection m_compoundInputs;
		InputManager& m_inputManager;
	public:

	private:
		void ParseFile(std::ifstream& stream);
		
	public:
		InputProfile(InputManager& manager, const std::string& name, const std::filesystem::path& filePath);

		const std::string& GetName() const;

		const std::vector<const InputAction*> GetActions() const;
		const InputAction* TryGetInputAction(const std::string& name) const;

		const std::vector<const CompoundInput*> GetCompoundActions() const;
		const CompoundInput* TryGetCompoundInputAction(const std::string& name) const;

		const std::vector<InputAction*> GetActionsMutable();
		const std::vector<CompoundInput*> GetCompoundActionsMutable();

		std::string ToString() const;
	};
}
