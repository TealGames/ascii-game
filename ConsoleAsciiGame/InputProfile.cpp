#include "pch.hpp"
#include "InputProfile.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"
#include <fstream>
#include "StringUtil.hpp"
#include "InputManager.hpp"

namespace Input
{
	constexpr char HEADER_CHAR = '@';
	const std::string GENERAL_HEADER = "General";
	const std::string INPUT_HEADER = "Input";

	constexpr char COMPOUND_INPUT_IDENTIFIER = '>';

	InputProfile::InputProfile(InputManager& manager,const std::string& name, const std::filesystem::path& filePath) :
		m_name(name), m_actions(), m_compoundInputs(), m_inputManager(manager)
	{
		if (!Assert(this, std::filesystem::exists(filePath), std::format("Tried to create input profile from path: '{}' "
			"but that path is not valid", filePath.string()))) return;

		if (!Assert(this, filePath.has_filename(), std::format("Tried to create input profile from path: '{}' "
			"but that path does not lead to a file", filePath.string()))) return;

		std::ifstream stream(filePath);
		ParseFile(stream);

		Log(this, std::format("Successfully created profile: {}", ToString()));
	}

	void InputProfile::ParseFile(std::ifstream& stream)
	{
		std::string line = "";
		bool isReadingMainHeader = false;

		CompoundInputCollection::iterator currentCompoundIt = m_compoundInputs.begin();
		std::string inputName = "";
		std::string keybindName = "";

		while (std::getline(stream, line))
		{
			if (line.empty()) continue;
			std::string lineBefore = line;
			line = Utils::StringUtil(line).RemoveSpaces().ToString();
			LogWarning(this, std::format("COnverted line: {} -> {}", lineBefore, line));

			if (line == HEADER_CHAR + GENERAL_HEADER)
			{
				isReadingMainHeader = true;
				continue;
			}
			else if (line == HEADER_CHAR + INPUT_HEADER)
			{
				isReadingMainHeader = false;
				continue;
			}

			if (isReadingMainHeader)
			{
				continue;
			}

			const size_t colonIndex = line.find(':');
			if (!Assert(this, colonIndex != std::string::npos, std::format("Tried to parse input profile but current line: '{}' "
				"does not contain any colons identifying action name end", line))) return;

			inputName = line.substr(0, colonIndex);

			if (line.size() - 1 == colonIndex)
			{
				currentCompoundIt = m_compoundInputs.emplace(inputName, CompoundInput(inputName)).first;
				continue;
			}

			bool isCompoundKey = line[0] == COMPOUND_INPUT_IDENTIFIER;
			if (isCompoundKey) inputName = inputName.substr(1);

			size_t commaIndex = line.find(",");
			size_t keybindStartIndex = colonIndex + 1;
			size_t keybindEndIndex = line.size() - 1;
			if (commaIndex != std::string::npos) keybindEndIndex = commaIndex - 1;

			std::optional<DeviceType> currentDevice = std::nullopt;
			std::vector<const InputKey*> keybinds = {};
			const InputKey* currentInputKey = nullptr;
			std::any inputAsEnum;
			do
			{
				keybindName = line.substr(keybindStartIndex, keybindEndIndex - keybindStartIndex + 1);
				inputAsEnum.reset();

				//TODO: perhaps the profile.txt should implicity say what devide the input is for and/or
				//also add the option to list out the CORRESPONDING key for different devices 
				currentDevice = TryGetStringKeyDevice(keybindName, &inputAsEnum);
				if (!Assert(this, currentDevice.has_value(), 
					std::format("Could not deduce the device from keybind name: '{}' of input profile: '{}'", keybindName, m_name))) 
					return;

				if (currentDevice.value() == DeviceType::Keyboard)
				{
					currentInputKey = m_inputManager.GetInputKey(std::any_cast<KeyboardKey>(inputAsEnum));
				}
				else if (currentDevice.value() == DeviceType::Mouse)
				{
					currentInputKey = m_inputManager.GetInputKey(std::any_cast<MouseButton>(inputAsEnum));
				}
				else if (currentDevice.value() == DeviceType::Gamepad)
				{
					currentInputKey = m_inputManager.GetInputKey(std::any_cast<GamepadButton>(inputAsEnum));
				}
				else
				{
					LogError(this, std::format("Tried to convert input key: '{}' from input profile: '{}' "
						"to raylib enum but there no device actions for: {}", keybindName, m_name, Input::ToString(currentDevice.value())));
					return;
				}
				keybinds.push_back(currentInputKey);

				//We then try to find the comma again and move the start index past the last end index
				//(+2 to account for comma and next space) and get the next end index
				commaIndex = line.find(",");
				if (commaIndex != std::string::npos)
				{
					keybindStartIndex = keybindEndIndex + 2;
					keybindEndIndex = commaIndex - 1;
				}
			} 
			while (commaIndex != std::string::npos);

			if (isCompoundKey)
			{
				if (!Assert(this, currentCompoundIt != m_compoundInputs.end(),
					std::format("Tried to parse an input profile but at line: '{}' "
						"tried to add input to compound but there is not compound set", line))) 
					return;

				std::optional<InputDirection> actionAsDir = TryConvertStringToDirection(inputName);
				if (!Assert(this, actionAsDir.has_value(), std::format("Tried to parse compound input action: '{}' as direction "
					"for compound input: '{}' but it failed. All compound input actions must be valid directions",
					inputName, currentCompoundIt->first)))
					return;

				currentCompoundIt->second.AddEntry(actionAsDir.value(), InputAction(inputName, keybinds));
			}
			else
			{
				m_actions.emplace(inputName, InputAction(inputName, keybinds));
			}
		}
	}

	const std::string& InputProfile::GetName() const
	{
		return m_name;
	}

	const std::vector<const InputAction*> InputProfile::GetActions() const
	{
		std::vector<const InputAction*> actions = {};
		for (const auto& action : m_actions) actions.push_back(&action.second);
		return actions;
	}
	const InputAction* InputProfile::TryGetInputAction(const std::string& name) const
	{
		auto it = m_actions.find(name);
		if (it == m_actions.end()) return nullptr;

		return &(it->second);
	}

	const std::vector<const CompoundInput*> InputProfile::GetCompoundActions() const
	{
		std::vector<const CompoundInput*> actions = {};
		for (const auto& action : m_compoundInputs) actions.push_back(&action.second);
		return actions;
	}
	const CompoundInput* InputProfile::TryGetCompoundInputAction(const std::string& name) const
	{
		auto it = m_compoundInputs.find(name);
		if (it == m_compoundInputs.end()) return nullptr;

		return &(it->second);
	}

	const std::vector<InputAction*> InputProfile::GetActionsMutable()
	{
		std::vector<InputAction*> actions = {};
		for (auto& action : m_actions) actions.push_back(&action.second);
		return actions;
	}
	const std::vector<CompoundInput*> InputProfile::GetCompoundActionsMutable()
	{
		std::vector<CompoundInput*> actions = {};
		for (auto& action : m_compoundInputs) actions.push_back(&action.second);
		return actions;
	}

	std::string InputProfile::ToString() const
	{
		std::string output = "{";

		output += std::format("Name: {}", m_name);

		auto onlyActionKeys = Utils::GetValuesFromMap<std::string, InputAction>(m_actions.begin(), m_actions.end());
		auto onlyCompoundKeys= Utils::GetValuesFromMap<std::string, CompoundInput>(m_compoundInputs.begin(), m_compoundInputs.end());

		output += std::format(",Actions:{}", Utils::ToStringIterable<std::vector<InputAction>, InputAction>(onlyActionKeys));
		output += std::format(",CompooundActions:{}", Utils::ToStringIterable<std::vector<CompoundInput>, CompoundInput>(onlyCompoundKeys));
		output += "}";
		return output;
	}
}
