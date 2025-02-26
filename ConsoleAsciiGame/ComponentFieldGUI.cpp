#include "pch.hpp"
#include "ComponentFieldGUI.hpp"
#include "ScreenPosition.hpp"
#include "RaylibUtils.hpp"
#include "Debug.hpp"
#include "ComponentGUI.hpp"
#include "EntityGUI.hpp"
#include "Entity.hpp"

constexpr static float TITLE_FONT_SIZE = 10;
constexpr static float FIELD_FONT_FACTOR = 0.8;
constexpr static int MAX_WIDTH = 100;
constexpr static int MAX_HEIGHT = 100;

constexpr static float FONT_SIZE = 10;
constexpr static float FONT_SPACING = 10;

const static ScreenPosition MAX_INPUT_FIELD_SIZE = { 100, 100 };

ComponentFieldGUI::ComponentFieldGUI(const Input::InputManager& inputManager, const ComponentGUI& componentGUI, ComponentField& field)
	: m_fieldInfo(field), m_inputFields(), m_inputManager(inputManager), m_componentGUI(&componentGUI)
{
	/*if (m_fieldInfo.IsCurrentType<Utils::Point2D>()) 
		Assert(false, std::format("Tried to create field but wtih value: {}", std::get<Utils::Point2D*>(m_fieldInfo.m_Value)->ToString()));*/
	/*if (m_fieldInfo.IsCurrentType<Utils::Point2D>() && GetComponentGUISafe().GetEntityGUISafe().GetEntity().m_Name== "player")*/
	//Assert(false, std::format("Tried to create field but wtih value: {}", std::get<Utils::Point2D*>(m_fieldInfo.m_Value)->ToString()));

	InputFieldFlag fieldFlags = InputFieldFlag::None;
	GUISettings guiSettings = GUISettings(MAX_INPUT_FIELD_SIZE, GRAY, TextGUISettings(WHITE, TextGUIFontSize::ParentArea, FIELD_FONT_FACTOR, 0));
	/*Assert(false, std::format("Creating gui settings for field: {} are: {}", 
		std::to_string(guiSettings.m_TextSettings.m_FontSizeParentAreaFactor), std::to_string(guiSettings.m_TextSettings.GetFontSize({10, 10}))));*/

	if (m_fieldInfo.IsCurrentType<int>())
	{
		m_inputFields.push_back(InputField(m_inputManager, InputFieldType::Integer, fieldFlags, guiSettings));
	}
	else if (m_fieldInfo.IsCurrentType<float>())
	{
		m_inputFields.push_back(InputField(m_inputManager, InputFieldType::Float, fieldFlags, guiSettings));
	}
	else if (m_fieldInfo.IsCurrentType<std::string>())
	{
		m_inputFields.push_back(InputField(m_inputManager, InputFieldType::String, fieldFlags, guiSettings));
	}
	else if (m_fieldInfo.IsCurrentType<Utils::Point2D>())
	{
		for (int i = 0; i < 2; i++)
		{
			m_inputFields.push_back(InputField(m_inputManager, InputFieldType::Float, fieldFlags, guiSettings));
		}
	}
	else
	{
		LogError(this, std::format("Tried to construct component field GUI for property: '{}' "
			"but could not find any actions for its type: {}", m_fieldInfo.m_FieldName, m_fieldInfo.GetCurrentType().name()));
	}
}

ComponentFieldGUI::~ComponentFieldGUI()
{
	//LogError("Destroyed compoent field GUI");
}

void ComponentFieldGUI::SetFieldToInternal()
{
	//Assert(false, std::format("Type for field is: {}", m_fieldInfo.GetCurrentType().name()));
	if (m_fieldInfo.IsCurrentType<int>())
	{
		m_inputFields[0].OverrideInput(std::to_string(*(m_fieldInfo.TryGetValue<int>())));
	}
	else if (m_fieldInfo.IsCurrentType<float>())
	{
		m_inputFields[0].OverrideInput(std::to_string(*(m_fieldInfo.TryGetValue<float>())));
	}
	else if (m_fieldInfo.IsCurrentType<std::string>())
	{
		//Assert(false, "POOP");
		const std::string* str = m_fieldInfo.TryGetValue<std::string>();
		//Assert(false, std::format("String FOR COMPOENNT FIELD IS: {}", str == nullptr ? "NULL" : *str));
		m_inputFields[0].OverrideInput(*str);
	}
	else if (m_fieldInfo.IsCurrentType<Utils::Point2D>())
	{
		const Utils::Point2D* point = m_fieldInfo.TryGetValue<Utils::Point2D>();
		//Assert(false, std::format("REACHED COMPENZTN FIELD POINT: {}",point == nullptr ? "NULL" : point->ToString()));

		m_inputFields[0].OverrideInput(std::to_string(point->m_X));
		m_inputFields[1].OverrideInput(std::to_string(point->m_Y));

		//Assert(false, std::format("HAS PINT: {} Input field input is now: {} should be: {}", std::to_string(point!=nullptr), m_inputFields[0].GetInput(), std::to_string(point->m_X)));
		//Assert(false, std::format("Input field input is now: {} should be: {}", m_inputFields[1].GetInput(), std::to_string(point->m_Y)));
	}
	else
	{
		LogError(this, std::format("Tried to set field to internal value for property: '{}' "
			"but could not find any actions for its type", m_fieldInfo.m_FieldName));
		return;
	}

	for (auto& field : m_inputFields)
	{
		LogError("Set field submit action");
		field.SetSubmitAction([this](std::string input) -> void
			{
				LogError(std::format("Setting internal input for field with str: {} type: {} is point: {}",
					input, m_fieldInfo.GetCurrentType().name(), std::to_string(m_fieldInfo.IsCurrentType<Utils::Point2D>())));

				for (auto& inputField : m_inputFields)
				{
					LogError(std::format("Found field: {} that belongs to type: {}", ToString(inputField.GetFieldType()), m_fieldInfo.GetCurrentType().name()));
				}

				SetInternalWithInput();
			});
	}
}
void ComponentFieldGUI::SetInternalWithInput()
{
	LogError(std::format("Getting internal input and setting it. is point: {} field name: {} is point: {}", 
		m_fieldInfo.GetCurrentType().name(), m_fieldInfo.m_FieldName, std::to_string(m_fieldInfo.IsCurrentType<Utils::Point2D>())));

	if (m_fieldInfo.IsCurrentType<int>())
	{
		LogError("REAHCED INT");
		//LogError(std::format("Setting internal value with: {}", std::to_string(m_inputFields[0].GetIntInput())));
		m_fieldInfo.TrySetValue<int>(m_inputFields[0].GetIntInput());
	}
	else if (m_fieldInfo.IsCurrentType<float>())
	{
		LogError("REAHCED FLOAT");
		m_fieldInfo.TrySetValue<float>(m_inputFields[0].GetFloatInput());
	}
	else if (m_fieldInfo.IsCurrentType<std::string>())
	{
		LogError("REAHCED STRINGF");
		m_fieldInfo.TrySetValue<std::string>(m_inputFields[0].GetInput());
	}
	else if (m_fieldInfo.IsCurrentType<Utils::Point2D>())
	{
		//m_inputFields[0].GetFloatInput(), m_inputFields[1].GetFloatInput()
		LogError("REAHCED POINT");
		m_fieldInfo.TrySetValue<Utils::Point2D>({ m_inputFields[0].GetFloatInput(), m_inputFields[1].GetFloatInput() });
		//Assert(false, std::format("REACHED COMPENZTN FIELD POINT: {}",point == nullptr ? "NULL" : point->ToString()));

		//Assert(false, std::format("HAS PINT: {} Input field input is now: {} should be: {}", std::to_string(point!=nullptr), m_inputFields[0].GetInput(), std::to_string(point->m_X)));
		//Assert(false, std::format("Input field input is now: {} should be: {}", m_inputFields[1].GetInput(), std::to_string(point->m_Y)));
	}
	else
	{
		LogError("REAHCED ERROR");
		LogError(this, std::format("Tried to set internal value with input for property: '{}' "
			"but could not find any actions for its type", m_fieldInfo.m_FieldName));
	}
}

void ComponentFieldGUI::Update()
{
	for (auto& inputField : m_inputFields)
	{
		inputField.Update();
		LogError(std::format("Found field: {} that belongs to type: {}", ToString(inputField.GetFieldType()), m_fieldInfo.GetCurrentType().name()));
	}
	//TODO: internal should only be set when enter pressed on input
	//SetInternalWithInput();

	SetFieldToInternal();
	//TODO: update the values in the field
}

ScreenPosition ComponentFieldGUI::Render(const RenderInfo& renderInfo)
{
	Vector2 currentPos = RaylibUtils::ToRaylibVector(renderInfo.m_TopLeftPos);
	const Vector2 textSize = MeasureTextEx(GetGlobalFont(), m_fieldInfo.m_FieldName.c_str(), TITLE_FONT_SIZE, DEBUG_INFO_CHAR_SPACING.m_X);
	//Assert(false, std::format("Field name: {}", m_fieldInfo.m_FieldName));
	//return {(int)currentPos.x, (int)currentPos.y};

	DrawTextEx(GetGlobalFont(), m_fieldInfo.m_FieldName.c_str(), currentPos, TITLE_FONT_SIZE, DEBUG_INFO_CHAR_SPACING.m_X, WHITE);

	currentPos.y += textSize.y;

	ScreenPosition inputFieldSizeUsed = {};
	ScreenPosition inputFieldSpace = { static_cast<int>(renderInfo.m_RenderSize.m_X / m_inputFields.size()), 
									   static_cast<int>(renderInfo.m_RenderSize.m_Y-(renderInfo.m_TopLeftPos.m_Y- currentPos.y))};

	for (auto& field : m_inputFields)
	{
		inputFieldSizeUsed= field.Render(RenderInfo({ static_cast<int>(currentPos.x), static_cast<int>(currentPos.y)}, inputFieldSpace));
		currentPos.x += inputFieldSizeUsed.m_X;
	}
	return { renderInfo.m_RenderSize.m_X, static_cast<int>(renderInfo.m_TopLeftPos.m_Y- currentPos.y + inputFieldSpace.m_Y)};
}

const ComponentField& ComponentFieldGUI::GetFieldInfo() const
{
	return m_fieldInfo;
}

const ComponentGUI& ComponentFieldGUI::GetComponentGUISafe() const
{
	if (!Assert(this, m_componentGUI != nullptr, std::format("Tried to get component GUI "
		"from a field named : '{}'", m_fieldInfo.ToString())))
	{
		throw std::invalid_argument("Failed to retrieve entity gui");
	}
	return *m_componentGUI;
}
