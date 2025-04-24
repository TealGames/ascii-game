#include "pch.hpp"
#include "InputProfileAsset.hpp"
#include <fstream>
#include "InputManager.hpp"
#include "IOHandler.hpp"

const std::string InputProfileAsset::EXTENSION = ".input";

static constexpr char HEADER_CHAR = '@';
static const std::string GENERAL_HEADER = "General";
static const std::string INPUT_HEADER = "Input";

static constexpr char COMPOUND_INPUT_IDENTIFIER = '>';

InputProfileAsset::InputProfileAsset(const std::filesystem::path& path)
	: Asset(path, true), m_profile(std::nullopt), m_inputManager(nullptr)
{
	if (!Assert(this, IO::DoesPathHaveExtension(path, EXTENSION), std::format("Tried to create a input profile asset from path:'{}' "
		"but it does not have required extension:'{}'", EXTENSION)))
		return;
}

Input::InputManager& InputProfileAsset::GetInputManager()
{
	if (!Assert(this, m_inputManager!=nullptr, std::format("Tried to retrieve scene MUTABLE from scene asset:{} "
		"but its asset has not been created yet due to dependencies for this asset not initialized", ToString())))
	{
		throw std::invalid_argument("Invalid scene asset dependency");
	}

	//LogError(std::format("Scene has value:{}", m_scene.value().ToString()));
	return *m_inputManager;
}

void InputProfileAsset::SetDependencies(Input::InputManager& input)
{
	m_profile = Input::InputProfile(input, GetName());
	m_inputManager = &input;
	MarkDependenciesSet();

	UpdateAssetFromFile();
}

void InputProfileAsset::UpdateAssetFromFile()
{
	std::ifstream stream(GetPath());

	std::string line = "";
	bool isReadingMainHeader = false;

	//Input::CompoundInputCollection::iterator currentCompoundIt = m_compoundInputs.begin();
	Input::CompoundInput currentCompoundInput = Input::CompoundInput();
	std::string inputName = "";
	std::string keybindName = "";

	while (std::getline(stream, line))
	{
		if (line.empty()) continue;
		//std::string lineBefore = line;
		line = Utils::StringUtil(line).RemoveSpaces().ToString();
		//LogWarning(this, std::format("COnverted line: {} -> {}", lineBefore, line));
		//LogError(std::format("Current line:'{}' header+genrall:'{}'", line, HEADER_CHAR + GENERAL_HEADER));

		if (line == HEADER_CHAR + GENERAL_HEADER)
		{
			//LogError("Found general");
			isReadingMainHeader = true;
			continue;
		}
		else if (line == HEADER_CHAR + INPUT_HEADER)
		{
			//LogError("Found input");
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
			//If we get here it means we have found a new compound so we should add it since it should be
			//finished by this point (entries are 0 if no compound was found)
			if (currentCompoundInput.GetEntriesCount() > 0)
			{
				GetProfileMutable().TryAddCompoundAction(currentCompoundInput);
			}

			currentCompoundInput = Input::CompoundInput(inputName);
			continue;
		}

		bool isCompoundKey = line[0] == COMPOUND_INPUT_IDENTIFIER;
		if (isCompoundKey) inputName = inputName.substr(1);

		size_t commaIndex = line.find(",");
		size_t keybindStartIndex = colonIndex + 1;
		size_t keybindEndIndex = line.size() - 1;
		if (commaIndex != std::string::npos) keybindEndIndex = commaIndex - 1;

		std::optional<Input::DeviceType> currentDevice = std::nullopt;
		std::vector<const Input::InputKey*> keybinds = {};
		const Input::InputKey* currentInputKey = nullptr;
		std::any inputAsEnum;
		do
		{
			keybindName = line.substr(keybindStartIndex, keybindEndIndex - keybindStartIndex + 1);
			inputAsEnum.reset();

			//TODO: perhaps the profile.txt should implicity say what devide the input is for and/or
			//also add the option to list out the CORRESPONDING key for different devices 
			currentDevice = Input::TryGetStringKeyDevice(keybindName, &inputAsEnum);
			if (!Assert(this, currentDevice.has_value(),
				std::format("Could not deduce the device from keybind name: '{}' of input profile: '{}'", keybindName, GetName())))
				return;

			if (currentDevice.value() == Input::DeviceType::Keyboard)
			{
				currentInputKey = GetInputManager().GetInputKey(std::any_cast<KeyboardKey>(inputAsEnum));
			}
			else if (currentDevice.value() == Input::DeviceType::Mouse)
			{
				currentInputKey = GetInputManager().GetInputKey(std::any_cast<MouseButton>(inputAsEnum));
			}
			else if (currentDevice.value() == Input::DeviceType::Gamepad)
			{
				currentInputKey = GetInputManager().GetInputKey(std::any_cast<GamepadButton>(inputAsEnum));
			}
			else
			{
				LogError(this, std::format("Tried to convert input key: '{}' from input profile: '{}' "
					"to raylib enum but there no device actions for: {}", keybindName, GetName(), Input::ToString(currentDevice.value())));
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
		} while (commaIndex != std::string::npos);

		if (isCompoundKey)
		{
			/*if (!Assert(this, currentCompoundIt != m_compoundInputs.end(),
				std::format("Tried to parse an input profile but at line: '{}' "
					"tried to add input to compound but there is not compound set", line)))
				return;*/

			std::optional<Input::InputDirection> actionAsDir = Input::TryConvertStringToDirection(inputName);
			if (!Assert(this, actionAsDir.has_value(), std::format("Tried to parse compound input action: '{}' as direction "
				"for compound input: '{}' but it failed. All compound input actions must be valid directions",
				inputName, currentCompoundInput.GetName())))
				return;

			currentCompoundInput.AddEntry(actionAsDir.value(), Input::InputAction(inputName, keybinds));
		}
		else
		{
			GetProfileMutable().TryAddAction(inputName, keybinds);
			//m_actions.emplace(inputName, InputAction(inputName, keybinds));
		}
	}

	//Just in case to ensure if we had final compound it was stored
	if (currentCompoundInput.GetEntriesCount() > 0)
	{
		GetProfileMutable().TryAddCompoundAction(currentCompoundInput);
	}

	Log(this, std::format("Successfully created profile: {}", GetProfileMutable().ToString()));
}

void InputProfileAsset::SaveToPath(const std::filesystem::path& path)
{
	//TODO: implement
}

Input::InputProfile& InputProfileAsset::GetProfileMutable()
{
	if (!Assert(this, m_profile.has_value(), std::format("Tried to retrieve input MUTABLE from input asset:{} "
		"but its asset has not been created yet due to dependencies for this asset not initialized", ToString())))
		throw std::invalid_argument("Invalid input asset dependency");

	return m_profile.value();
}
const Input::InputProfile& InputProfileAsset::GetProfile() const
{
	if (!Assert(this, m_profile.has_value(), std::format("Tried to retrieve input from input asset:{} "
		"but its asset has not been created yet due to dependencies for this asset not initialized", ToString())))
		throw std::invalid_argument("Invalid input asset dependency");

	return m_profile.value();
}


