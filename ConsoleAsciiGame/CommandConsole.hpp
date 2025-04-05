#pragma once
#include <unordered_map>
#include <string>
#include "CommandPrompt.hpp"
#include "raylib.h"
#include <memory>
#include <queue>
#include <chrono> 
#include "InputManager.hpp"
#include "InputField.hpp"
#include "IRenderable.hpp"

using Time = std::chrono::time_point<std::chrono::high_resolution_clock>;
using PromptCollection = std::unordered_map<std::string, std::vector<ICommandPrompt*>>;

struct ConsoleOutputMessage
{
	std::string m_Message = "";
	Color m_Color = {};
	Time m_StartTime = {};
};

enum class ConsoleOutputMessageType
{
	Default,
	Error,
	Success,
};

class GUISelectorManager;

class CommandConsole : IRenderable
{
private:
	/// <summary>
	/// The main prompt name with the prompt info
	/// </summary>
	PromptCollection m_prompts;

	InputField m_inputField;
	std::vector<ConsoleOutputMessage> m_outputMessages;

	const Input::InputManager& m_inputManager;
	bool m_isEnabled;

public:
	static constexpr char COMMAND_CHAR= '/';
	static constexpr KeyboardKey TOGGLE_COMMAND_CONSOLE_KEY = KEY_TAB;

private:
	std::string FormatPromptName(const std::string& name);
	PromptCollection::iterator TryGetIteratorForPromptName(const std::string& name);
	std::string GetDocumentationForPrompt(PromptCollection::iterator& promptIt, const size_t& index) const;

	bool TryInvokePrompt();
	Color GetColorFromMessageType(const ConsoleOutputMessageType& message);
	
public:
	CommandConsole(const Input::InputManager& input, GUISelectorManager& selector);

	//void SetActiveConsole(CommandConsole& console);

	void AddPrompt(ICommandPrompt* prompt);
	bool HasPrompt(const std::string& promptName);
	void DeletePrompts();

	void Update();
	void TryRender();
	ScreenPosition Render(const RenderInfo& renderInfo) override;

	bool IsEnabled() const;

	void ResetInput();
	//std::string GetInput() const;

	std::vector<std::string> GetPromptDocumentation(const std::string& promptName);
	std::vector<std::string> GetPromptDocumentationAll();

	void LogOutputMessage(const std::string& message, const ConsoleOutputMessageType& messageType);

	/// <summary>
	/// Will log the messages and will show all of them even if a limit would have
	/// caused a message to be hidden
	/// </summary>
	/// <param name="messages"></param>
	void LogOutputMessagesUnrestricted(const std::vector<std::string>& messages, const ConsoleOutputMessageType& messageType);

	const std::vector<ConsoleOutputMessage>& GetOutputMessages() const;
};

