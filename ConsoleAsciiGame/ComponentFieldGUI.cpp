#include "pch.hpp"
#include "ComponentFieldGUI.hpp"
#include "ScreenPosition.hpp"
#include "RaylibUtils.hpp"
#include "Debug.hpp"
#include "ComponentGUI.hpp"
#include "EntityGUI.hpp"
#include "Entity.hpp"
#include "GUISelectorManager.hpp"
#include "EntityEditorGUI.hpp"
#include "Vec2.hpp"

constexpr static float TITLE_FONT_SIZE = 10;
constexpr static float FIELD_FONT_FACTOR = 0.8;

constexpr static float FONT_SIZE = 10;
constexpr static float FONT_SPACING = 10;

const static ScreenPosition MAX_INPUT_FIELD_SIZE = { 100, 20 };

ComponentFieldGUI::ComponentFieldGUI(const Input::InputManager& inputManager, GUISelectorManager& selector, 
	const ComponentGUI& componentGUI, ComponentField& field)
	: m_fieldInfo(&field), m_inputFields(), m_checkbox(), m_inputManager(&inputManager), m_componentGUI(&componentGUI), 
	m_fieldNameText(GetFieldInfo().m_FieldName, FontData(TITLE_FONT_SIZE, GetGlobalFont()), 
		EntityEditorGUI::EDITOR_CHAR_SPACING.m_X, EntityEditorGUI::EDITOR_SECONDARY_COLOR)
{
	/*if (m_fieldInfo.IsCurrentType<Vec2>()) 

		Assert(false, std::format("Tried to create field but wtih value: {}", std::get<Vec2*>(m_fieldInfo.m_Value)->ToString()));*/
	/*if (m_fieldInfo.IsCurrentType<Vec2>() && GetComponentGUISafe().GetEntityGUISafe().GetEntity().m_Name== "player")*/
	//Assert(false, std::format("Tried to create field but wtih value: {}", std::get<Vec2*>(m_fieldInfo.m_Value)->ToString()));

	InputFieldFlag fieldFlags = InputFieldFlag::None;
	if (GetFieldInfo().IsReadonly()) fieldFlags |= InputFieldFlag::UserUIReadonly;

	GUISettings fieldSettings = GUISettings(MAX_INPUT_FIELD_SIZE, EntityEditorGUI::EDITOR_SECONDARY_COLOR, 
		TextGUISettings(EntityEditorGUI::EDITOR_TEXT_COLOR, FontData(0, GetGlobalFont()), EntityEditorGUI::EDITOR_CHAR_SPACING.m_X, TextAlignment::Center, FIELD_FONT_FACTOR));
	/*Assert(false, std::format("Creating gui settings for field: {} are: {}", 
		std::to_string(guiSettings.m_TextSettings.m_FontSizeParentAreaFactor), std::to_string(guiSettings.m_TextSettings.GetFontSize({10, 10}))));*/

	if (GetFieldInfo().IsCurrentType<int>() || GetFieldInfo().IsCurrentType<std::uint8_t>())
	{
		m_inputFields.push_back(InputField(GetInputManager(), selector, InputFieldType::Integer, fieldFlags, fieldSettings));
	}
	else if (GetFieldInfo().IsCurrentType<float>())
	{
		m_inputFields.push_back(InputField(GetInputManager(), selector, InputFieldType::Float, fieldFlags, fieldSettings));
	}
	else if (GetFieldInfo().IsCurrentType<bool>())
	{
		m_checkbox = ToggleGUI(selector, false, fieldSettings);
	}
	else if (GetFieldInfo().IsCurrentType<std::string>())
	{
		m_inputFields.push_back(InputField(GetInputManager(), selector, InputFieldType::String, fieldFlags, fieldSettings));
	}
	else if (GetFieldInfo().IsCurrentType<Vec2>())
	{
		for (int i = 0; i < 2; i++)
		{
			m_inputFields.push_back(InputField(GetInputManager(), selector, InputFieldType::Float, fieldFlags, fieldSettings));
		}
	}
	else if (GetFieldInfo().IsCurrentType<Vec2Int>())
	{
		for (int i = 0; i < 2; i++)
		{
			m_inputFields.push_back(InputField(GetInputManager(), selector, InputFieldType::Integer, fieldFlags, fieldSettings));
		}
	}
	else
	{
		LogError(this, std::format("Tried to construct component field GUI for property: '{}' "
			"but could not find any actions for its type: {}", GetFieldInfo().m_FieldName, GetFieldInfo().GetCurrentType().name()));
	}
}

ComponentFieldGUI::~ComponentFieldGUI()
{
	//LogError("Destroyed compoent field GUI");
}

const Input::InputManager& ComponentFieldGUI::GetInputManager() const
{
	if (!Assert(this, m_inputManager != nullptr, std::format("Tried to get Input Manager but it is null")))
		throw std::invalid_argument("Invalid input manager state");

	return *m_inputManager;
}
ComponentField& ComponentFieldGUI::GetFieldInfo()
{
	if (!Assert(this, m_fieldInfo != nullptr, std::format("Tried to get field info but it is NULL")))
		throw std::invalid_argument("Invalid field info");

	return *m_fieldInfo;
}

void ComponentFieldGUI::SetFieldToInternal()
{
	//Assert(false, std::format("Type for field is: {}", m_fieldInfo.GetCurrentType().name()));
	if (GetFieldInfo().IsCurrentType<int>())
	{
		m_inputFields[0].OverrideInput(std::to_string(*(GetFieldInfo().TryGetValue<int>())));
	}
	else if (GetFieldInfo().IsCurrentType<std::uint8_t>())
	{
		m_inputFields[0].OverrideInput(std::to_string(*(GetFieldInfo().TryGetValue<std::uint8_t>())));
	}
	else if (GetFieldInfo().IsCurrentType<float>())
	{
		m_inputFields[0].OverrideInput(std::to_string(*(GetFieldInfo().TryGetValue<float>())));
	}
	else if (GetFieldInfo().IsCurrentType<bool>())
	{
		m_checkbox.SetValue(*(GetFieldInfo().TryGetValue<bool>()));
	}
	else if (GetFieldInfo().IsCurrentType<std::string>())
	{
		//Assert(false, "POOP");
		const std::string* str = GetFieldInfo().TryGetValue<std::string>();
		//Assert(false, std::format("String FOR COMPOENNT FIELD IS: {}", str == nullptr ? "NULL" : *str));
		m_inputFields[0].OverrideInput(*str);
	}
	else if (GetFieldInfo().IsCurrentType<Vec2>())
	{
		const Vec2* point = GetFieldInfo().TryGetValue<Vec2>();
		//Assert(false, std::format("REACHED COMPENZTN FIELD POINT: {}",point == nullptr ? "NULL" : point->ToString()));

		m_inputFields[0].OverrideInput(std::to_string(point->m_X));
		m_inputFields[1].OverrideInput(std::to_string(point->m_Y));

		//Assert(false, std::format("HAS PINT: {} Input field input is now: {} should be: {}", std::to_string(point!=nullptr), m_inputFields[0].GetInput(), std::to_string(point->m_X)));
		//Assert(false, std::format("Input field input is now: {} should be: {}", m_inputFields[1].GetInput(), std::to_string(point->m_Y)));
	}
	else if (GetFieldInfo().IsCurrentType<Vec2Int>())
	{
		const Vec2Int* point = GetFieldInfo().TryGetValue<Vec2Int>();
		m_inputFields[0].OverrideInput(std::to_string(point->m_X));
		m_inputFields[1].OverrideInput(std::to_string(point->m_Y));
	}
	else
	{
		LogError(this, std::format("Tried to set field to internal value for property: '{}' "
			"but could not find any actions for its type", GetFieldInfo().m_FieldName));
		return;
	}

	//TODO: this should not be called on set field to internal since it gets called every frame
	if (!m_inputFields.empty())
	{
		for (auto& field : m_inputFields)
		{
			//LogError("Set field submit action");
			field.SetSubmitAction([this](std::string input) -> void
				{
					//LogError(std::format("Setting internal input for field with str: {} type: {} is point: {}",
					//	input, m_fieldInfo.GetCurrentType().name(), std::to_string(m_fieldInfo.IsCurrentType<Vec2>())));

					//for (auto& inputField : m_inputFields)
					//{
					//	LogError(std::format("Found field: {} that belongs to type: {}", ToString(inputField.GetFieldType()), m_fieldInfo.GetCurrentType().name()));
					//}

					SetInternalWithInput();
				});
		}
	}
	else m_checkbox.SetValueSetAction([this](bool isChecked)-> void
		{
			SetInternalWithInput();
		});
}
void ComponentFieldGUI::SetInternalWithInput()
{
	if (GetFieldInfo().IsReadonly()) return;

	/*LogError(std::format("Getting internal input and setting it. is point: {} field name: {} is point: {}", 
		m_fieldInfo.GetCurrentType().name(), m_fieldInfo.m_FieldName, std::to_string(m_fieldInfo.IsCurrentType<Vec2>())));*/

	if (GetFieldInfo().IsCurrentType<int>())
	{
		//LogError("REAHCED INT");
		//LogError(std::format("Setting internal value with: {}", std::to_string(m_inputFields[0].GetIntInput())));
		GetFieldInfo().TrySetValue<int>(m_inputFields[0].GetIntInput());
	}
	else if (GetFieldInfo().IsCurrentType<std::uint8_t>())
	{
		//LogError("REAHCED UNISGNED INT");
		//LogError(std::format("Setting internal value with: {}", std::to_string(m_inputFields[0].GetIntInput())));
		std::uint8_t convertedValue = static_cast<std::uint8_t>(std::abs(m_inputFields[0].GetIntInput()));
		GetFieldInfo().TrySetValue<std::uint8_t>(convertedValue);
	}
	else if (GetFieldInfo().IsCurrentType<float>())
	{
		//LogError("REAHCED FLOAT");
		GetFieldInfo().TrySetValue<float>(m_inputFields[0].GetFloatInput());
	}
	else if (GetFieldInfo().IsCurrentType<bool>())
	{
		GetFieldInfo().TrySetValue<bool>(m_checkbox.IsToggled());
	}
	else if (GetFieldInfo().IsCurrentType<std::string>())
	{
		//LogError("REAHCED STRINGF");
		GetFieldInfo().TrySetValue<std::string>(m_inputFields[0].GetInput());
	}
	else if (GetFieldInfo().IsCurrentType<Vec2>())
	{
		//m_inputFields[0].GetFloatInput(), m_inputFields[1].GetFloatInput()
		//LogError("REAHCED POINT");
		GetFieldInfo().TrySetValue<Vec2>({ m_inputFields[0].GetFloatInput(), m_inputFields[1].GetFloatInput() });
		//Assert(false, std::format("REACHED COMPENZTN FIELD POINT: {}",point == nullptr ? "NULL" : point->ToString()));

		//Assert(false, std::format("HAS PINT: {} Input field input is now: {} should be: {}", std::to_string(point!=nullptr), m_inputFields[0].GetInput(), std::to_string(point->m_X)));
		//Assert(false, std::format("Input field input is now: {} should be: {}", m_inputFields[1].GetInput(), std::to_string(point->m_Y)));
	}
	else if (GetFieldInfo().IsCurrentType<Vec2Int>())
	{
		//m_inputFields[0].GetFloatInput(), m_inputFields[1].GetFloatInput()
		//LogError("REAHCED POINT");
		GetFieldInfo().TrySetValue<Vec2Int>({ m_inputFields[0].GetIntInput(), m_inputFields[1].GetIntInput() });
		//Assert(false, std::format("REACHED COMPENZTN FIELD POINT: {}",point == nullptr ? "NULL" : point->ToString()));

		//Assert(false, std::format("HAS PINT: {} Input field input is now: {} should be: {}", std::to_string(point!=nullptr), m_inputFields[0].GetInput(), std::to_string(point->m_X)));
		//Assert(false, std::format("Input field input is now: {} should be: {}", m_inputFields[1].GetInput(), std::to_string(point->m_Y)));
	}
	else
	{
		//LogError("REAHCED ERROR");
		LogError(this, std::format("Tried to set internal value with input for property: '{}' "
			"but could not find any actions for its type", GetFieldInfo().m_FieldName));
	}
}

void ComponentFieldGUI::Update()
{
	if (!m_inputFields.empty())
	{
		for (auto& inputField : m_inputFields)
		{
			inputField.Update();
			//LogError(std::format("Found field: {} that belongs to type: {}", ToString(inputField.GetFieldType()), m_fieldInfo.GetCurrentType().name()));
		}
	}
	else m_checkbox.Update();
	//TODO: internal should only be set when enter pressed on input
	//SetInternalWithInput();

	//NOTE: yes this might be bad to set to internal value every update, but
	//we do not know when the value internally can change, so it must be updated
	//every frame
	SetFieldToInternal();
	//TODO: update the values in the field
}

/*
ScreenPosition ComponentFieldGUI::Render(const RenderInfo& renderInfo)
{
	Vector2 currentPos = RaylibUtils::ToRaylibVector(renderInfo.m_TopLeftPos);
	const Vector2 textSize = MeasureTextEx(GetGlobalFont(), m_fieldInfo.m_FieldName.c_str(), TITLE_FONT_SIZE, DEBUG_INFO_CHAR_SPACING.m_X);
	//Assert(false, std::format("Field name: {}", m_fieldInfo.m_FieldName));
	//return {(int)currentPos.x, (int)currentPos.y};

	DrawTextEx(GetGlobalFont(), m_fieldInfo.m_FieldName.c_str(), currentPos, TITLE_FONT_SIZE, DEBUG_INFO_CHAR_SPACING.m_X, EntityEditorGUI::EDITOR_SECONDARY_COLOR);
	currentPos.y += textSize.y;

	const int heightLeft = renderInfo.m_RenderSize.m_Y - (currentPos.y - renderInfo.m_TopLeftPos.m_Y);
	if (!m_inputFields.empty())
	{
		ScreenPosition inputFieldSizeUsed = {};
		ScreenPosition inputFieldSpace = { static_cast<int>(renderInfo.m_RenderSize.m_X / m_inputFields.size()),
										   std::min(heightLeft, MAX_INPUT_FIELD_SIZE.m_Y) };

		for (auto& field : m_inputFields)
		{
			inputFieldSizeUsed = field.Render(RenderInfo({ static_cast<int>(currentPos.x), 
														   static_cast<int>(currentPos.y) }, inputFieldSpace));
			currentPos.x += inputFieldSizeUsed.m_X;
		}
		currentPos.y += inputFieldSpace.m_Y;
	}
	else
	{
		//NOTE: unlike the fields which are big and need to be on new lines, we can render the name and checkbox on the same line
		ScreenPosition checkboxSize = m_checkbox.Render(RenderInfo(renderInfo.m_TopLeftPos,
															ScreenPosition{ renderInfo.m_RenderSize.m_X, heightLeft }));
		currentPos.y += checkboxSize.m_Y;
	}
	
	return { renderInfo.m_RenderSize.m_X, static_cast<int>(currentPos.y - renderInfo.m_TopLeftPos.m_Y) };
}
*/

ScreenPosition ComponentFieldGUI::SetupRender(const RenderInfo& renderInfo, Event<void>& renderActions)
{
	Vector2 currentPos = RaylibUtils::ToRaylibVector(renderInfo.m_TopLeftPos);
	const Vector2 textSize = RaylibUtils::ToRaylibVector(m_fieldNameText.CalculateSize(renderInfo)); 
	/*if (m_fieldInfo.m_FieldName=="Pos") Assert(false, std::format("Rendering field:{} top left:{} rendersize:{} text size:{} ({})", m_fieldInfo.m_FieldName, renderInfo.m_TopLeftPos.ToString(),
		renderInfo.m_RenderSize.ToString(), m_fieldNameText.CalculateSize(renderInfo).ToString(), RaylibUtils::ToString(textSize)));*/
	//const Vector2 textSize= MeasureTextEx(GetGlobalFont(), m_fieldInfo.m_FieldName.c_str(), TITLE_FONT_SIZE, DEBUG_INFO_CHAR_SPACING.m_X);

	renderActions.AddListener([this, textSize, renderInfo]() -> void
		{
			m_fieldNameText.Render(RenderInfo(renderInfo.m_TopLeftPos, {static_cast<int>(textSize.x), static_cast<int>(textSize.y)}));
			/*DrawTextEx(GetGlobalFont(), m_fieldInfo.m_FieldName.c_str(), currentPos, 
				TITLE_FONT_SIZE, DEBUG_INFO_CHAR_SPACING.m_X, EntityEditorGUI::EDITOR_SECONDARY_COLOR); */
		});
	currentPos.y += textSize.y;

	const int heightLeft = renderInfo.m_RenderSize.m_Y - (currentPos.y - renderInfo.m_TopLeftPos.m_Y);
	if (!m_inputFields.empty())
	{
		//ScreenPosition inputFieldSizeUsed = {};
		ScreenPosition inputFieldSpace = { static_cast<int>(renderInfo.m_RenderSize.m_X / m_inputFields.size()),
										   std::min(heightLeft, MAX_INPUT_FIELD_SIZE.m_Y) };

		for (auto& field : m_inputFields)
		{
			renderActions.AddListener([currentPos, inputFieldSpace, &field]() -> void
				{
					ScreenPosition size= field.Render(RenderInfo({ static_cast<int>(currentPos.x),
														   static_cast<int>(currentPos.y) }, inputFieldSpace)); 
					//LogError(std::format("Input field has size: {}", size.ToString()));
				});
			currentPos.x += inputFieldSpace.m_X;
		}
		currentPos.y += inputFieldSpace.m_Y;
	}
	else
	{
		//NOTE: unlike the fields which are big and need to be on new lines, we can render the name and checkbox on the same line
		renderActions.AddListener([this, currentPos, renderInfo, heightLeft, textSize]() -> void
			{
				m_checkbox.Render(RenderInfo(ScreenPosition(currentPos.x, currentPos.y),
					ScreenPosition{ renderInfo.m_RenderSize.m_X, heightLeft })).m_Y;
				//Assert(false, std::format("Rendering checkbox topL:{} currentPos:{}", renderInfo.m_TopLeftPos.ToString(), RaylibUtils::ToString(currentPos)));
				//Assert(false, std::format("Checkbox size:{}", checkboxSize.ToString()));
			});
		//currentPos.y += heightLeft;
	}

	return { renderInfo.m_RenderSize.m_X, static_cast<int>(currentPos.y - renderInfo.m_TopLeftPos.m_Y) };
}

const ComponentField& ComponentFieldGUI::GetFieldInfo() const
{
	return GetFieldInfo();
}

const ComponentGUI& ComponentFieldGUI::GetComponentGUISafe() const
{
	if (!Assert(this, m_componentGUI != nullptr, std::format("Tried to get component GUI "
		"from a field named : '{}'", GetFieldInfo().ToString())))
	{
		throw std::invalid_argument("Failed to retrieve entity gui");
	}
	return *m_componentGUI;
}
