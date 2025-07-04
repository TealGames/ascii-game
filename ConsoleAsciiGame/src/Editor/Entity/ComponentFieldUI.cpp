#include "pch.hpp"
#include "Editor/Entity/ComponentFieldUI.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Editor/Entity/ComponentUI.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UITransformData.hpp"
#include "ECS/Component/Types/UI/UIInputField.hpp"
#include "Core/Input/InputManager.hpp"
#include "ECS/Component/Types/UI/UIToggleComponent.hpp"
#include "ECS/Component/Types/Editor/UIColorPicker.hpp"
#include "Editor/EditorStyles.hpp"
#include "Utils/Data/Vec2.hpp"

constexpr static float TITLE_FONT_SIZE = 10;
constexpr static float FIELD_TEXT_FONT_FACTOR = 0.8;
constexpr static float INPUT_FIELD_TEXT_FONT_FACTOR = 0.8;

//constexpr static float FONT_SIZE = 10;
//constexpr static float FONT_SPACING = 10;

constexpr static float FIELD_IDENT = 0.05;
constexpr static float FIELD_SPACING_X = 0.05;

constexpr static float FIELD_WIDTH_PER_CHAR = 0.05;
constexpr static float FIELD_NAME_CHAR_NEW_LINE_THRESHOLD = 0.5;

ComponentFieldUI::ComponentFieldUI(const Input::InputManager& inputManager, PopupUIManager& popupManager,
	const ComponentUI& componentGUI, UILayout& parent)
	: m_inputManager(&inputManager), m_fieldInfo(nullptr), m_fields(), m_componentGUI(&componentGUI), m_fieldNameText(nullptr), m_guiLayout(nullptr)
{
	EntityData* layoutEntity = nullptr;
	std::tie(layoutEntity, m_guiLayout) = parent.CreateLayoutElement("FieldLauout");

	auto [fieldNameTextEntity, fieldNameTextTransform] = layoutEntity->CreateChildUI("FieldNameText");
	m_fieldNameText = &(fieldNameTextEntity->AddComponent(UITextComponent("", EditorStyles::GetTextStyleFactorSize(TextAlignment::CenterLeft, FIELD_TEXT_FONT_FACTOR))));
}

void ComponentFieldUI::SetupInputFields(UITransformData& fieldNameTextTransform, const bool fieldsStartNewLine)
{
	float currentFieldX = fieldsStartNewLine ? FIELD_IDENT : fieldNameTextTransform.GetRect().GetBottomRighttPos().GetX();
	const float fieldSizeX = (NormalizedPosition::MAX - currentFieldX - (FIELD_SPACING_X * (m_fields.size() - 1))) / m_fields.size();

	UIInputField* inputField = nullptr;
	UITransformData* inputFieldTransform = nullptr;
	for (auto& field : m_fields)
	{
		inputField = static_cast<UIInputField*>(field);
		inputFieldTransform = inputField->GetEntityMutable().TryGetComponentMutable<UITransformData>();

		inputField->SetSubmitAction([this](std::string input) -> void
			{
				SetInternalWithInput();
			});

		inputFieldTransform->SetSize(NormalizedPosition(fieldSizeX, fieldsStartNewLine ? 0.5 : 1));
		inputFieldTransform->SetTopLeftPos({ currentFieldX, fieldsStartNewLine ? 1 - fieldNameTextTransform.GetSize().GetY() : 1 });

		currentFieldX += inputFieldTransform->GetSize().GetX() + FIELD_SPACING_X;
	}

	//LogWarning(std::format("Field size:{}", std::to_string(FIELD_SPACING_X * (m_fields.size() - 1))));
}

ComponentFieldUI::~ComponentFieldUI()
{
	LogError("Destroyed compoent field GUI");
}

void ComponentFieldUI::SetField(ComponentField& field)
{
	m_fieldInfo = &field;

	m_fieldNameText->SetText(GetFieldInfo().m_FieldName);
	UITransformData& fieldNameTextTransform = *(m_fieldNameText->GetEntityMutable().TryGetComponentMutable<UITransformData>());
	EntityData& layoutEntity = m_guiLayout->GetEntityMutable();

	const float textWidthNorm = FIELD_WIDTH_PER_CHAR * m_fieldNameText->GetText().size();
	const bool fieldsStartNewLine = textWidthNorm >= FIELD_NAME_CHAR_NEW_LINE_THRESHOLD;
	fieldNameTextTransform.SetSize(NormalizedPosition(textWidthNorm, fieldsStartNewLine ? 0.5 : 1.0));
	fieldNameTextTransform.SetTopLeftPos({ FIELD_IDENT, 1 });

	InputFieldFlag fieldFlags = InputFieldFlag::None;
	if (GetFieldInfo().IsReadonly()) fieldFlags |= InputFieldFlag::UserUIReadonly;

	/*GUIStyle fieldSettings = GUIStyle(EditorStyles::EDITOR_SECONDARY_COLOR,
		TextGUIStyle(EditorStyles::EDITOR_TEXT_COLOR, FontProperties(0, EditorStyles::EDITOR_CHAR_SPACING.m_X, GetGlobalFont()), TextAlignment::Center, INPUT_FIELD_TEXT_FONT_FACTOR));*/
		/*Assert(false, std::format("Creating gui settings for field: {} are: {}",
			std::to_string(guiSettings.m_TextSettings.m_FontSizeParentAreaFactor), std::to_string(guiSettings.m_TextSettings.GetFontSize({10, 10}))));*/

	if (GetFieldInfo().IsCurrentType<int>() || GetFieldInfo().IsCurrentType<std::uint8_t>())
	{
		m_fields.emplace_back(std::get<2>(layoutEntity.CreateChildUI("IntField", UIInputField(GetInputManager(), InputFieldType::Integer, fieldFlags,
			EditorStyles::GetInputFieldStyle(TextAlignment::Center, INPUT_FIELD_TEXT_FONT_FACTOR)))));
		SetupInputFields(fieldNameTextTransform, fieldsStartNewLine);
	}
	else if (GetFieldInfo().IsCurrentType<float>())
	{
		m_fields.emplace_back(std::get<2>(layoutEntity.CreateChildUI("FloatField", UIInputField(GetInputManager(), InputFieldType::Float, fieldFlags,
			EditorStyles::GetInputFieldStyle(TextAlignment::Center, INPUT_FIELD_TEXT_FONT_FACTOR)))));
		SetupInputFields(fieldNameTextTransform, fieldsStartNewLine);
	}
	else if (GetFieldInfo().IsCurrentType<std::string>())
	{
		m_fields.emplace_back(std::get<2>(layoutEntity.CreateChildUI("StringField", UIInputField(GetInputManager(), InputFieldType::String, fieldFlags,
			EditorStyles::GetInputFieldStyle(TextAlignment::Center, INPUT_FIELD_TEXT_FONT_FACTOR)))));
		SetupInputFields(fieldNameTextTransform, fieldsStartNewLine);
	}
	else if (GetFieldInfo().IsCurrentType<Vec2>())
	{
		m_fields.reserve(2);
		for (int i = 0; i < 2; i++)
		{
			m_fields.emplace_back(std::get<2>(layoutEntity.CreateChildUI("FloatField", UIInputField(GetInputManager(), InputFieldType::Float, fieldFlags,
				EditorStyles::GetInputFieldStyle(TextAlignment::Center, INPUT_FIELD_TEXT_FONT_FACTOR)))));
		}
		SetupInputFields(fieldNameTextTransform, fieldsStartNewLine);
	}
	else if (GetFieldInfo().IsCurrentType<Vec2Int>())
	{
		m_fields.reserve(2);
		for (int i = 0; i < 2; i++)
		{
			m_fields.emplace_back(std::get<2>(layoutEntity.CreateChildUI("IntField", UIInputField(GetInputManager(), InputFieldType::Integer, fieldFlags,
				EditorStyles::GetInputFieldStyle(TextAlignment::Center, INPUT_FIELD_TEXT_FONT_FACTOR)))));
		}
		SetupInputFields(fieldNameTextTransform, fieldsStartNewLine);
	}
	else if (GetFieldInfo().IsCurrentType<bool>())
	{
		auto [toggleEntity, toggleTransform, toggle] = layoutEntity.CreateChildUI("BoolField", UIToggleComponent(false, EditorStyles::GetToggleStyle()));
		toggleTransform->SetSize({ 1, 1 });
		toggle->m_OnValueSet.AddListener([this](bool isChecked)-> void
			{
				//LogError("Fart and shit");
				SetInternalWithInput();
			});
		m_fields.emplace_back(toggle);
	}
	else if (GetFieldInfo().IsCurrentType<Utils::Color>())
	{
		auto [colorPickerEntity, colorPickerTransform, colorPicker] = layoutEntity.CreateChildUI("ColorField", UIColorPickerData(UIStyle()));

		const float fieldTopLeftX = fieldsStartNewLine ? FIELD_IDENT : fieldNameTextTransform.GetRect().GetBottomRighttPos().GetX();
		colorPickerTransform->SetSize(NormalizedPosition(NormalizedPosition::MAX - fieldTopLeftX, fieldsStartNewLine ? 0.5 : 1));
		colorPickerTransform->SetTopLeftPos({ fieldTopLeftX, fieldsStartNewLine ? 1 - fieldNameTextTransform.GetSize().GetY() : 1 });
		colorPicker->SetValueSetAction([this](Utils::Color color)-> void
			{
				SetInternalWithInput();
			});
		m_fields.emplace_back(colorPicker);
		//m_colorPicker.SetSettings(fieldSettings);
	}
	else
	{
		LogError(std::format("Tried to construct component field GUI for property: '{}' "
			"but could not find any actions for its type: {}", GetFieldInfo().m_FieldName, GetFieldInfo().GetCurrentType().name()));
	}

	//If we have 2 lines, we use the full max space, otherwise we use half
	if (fieldsStartNewLine) m_guiLayout->SetSize({ 1, 1 });
	else m_guiLayout->SetSize({ 1, 0.5 });

	//TODO: this feels like a very gimicky solution to the problem where many fields for a component usually results in the parent layout shrinking their x and y size
	//thus leading to fields that fit, but are very small in terms of their width. The solution was to fix their horizontal size so layout cannot change it
	m_guiLayout->SetFixed(true, false);

	SetFieldToInternal();
}

const Input::InputManager& ComponentFieldUI::GetInputManager() const
{
	if (!Assert(m_inputManager != nullptr, std::format("Tried to get Input Manager but it is null")))
		throw std::invalid_argument("Invalid input manager state");

	return *m_inputManager;
}
ComponentField& ComponentFieldUI::GetFieldInfo()
{
	if (!Assert(m_fieldInfo != nullptr, std::format("Tried to get field info but it is NULL")))
		throw std::invalid_argument("Invalid field info");

	return *m_fieldInfo;
}

void ComponentFieldUI::SetFieldToInternal()
{
	//Assert(false, std::format("Type for field is: {}", m_fieldInfo.GetCurrentType().name()));
	if (GetFieldInfo().IsCurrentType<int>())
	{
		static_cast<UIInputField*>(m_fields[0])->OverrideInput(std::to_string(*(GetFieldInfo().TryGetValue<int>())));
	}
	else if (GetFieldInfo().IsCurrentType<std::uint8_t>())
	{
		static_cast<UIInputField*>(m_fields[0])->OverrideInput(std::to_string(*(GetFieldInfo().TryGetValue<std::uint8_t>())));
	}
	else if (GetFieldInfo().IsCurrentType<float>())
	{
		static_cast<UIInputField*>(m_fields[0])->OverrideInput(std::to_string(*(GetFieldInfo().TryGetValue<float>())));
	}
	else if (GetFieldInfo().IsCurrentType<std::string>())
	{
		//Assert(false, "POOP");
		const std::string* str = GetFieldInfo().TryGetValue<std::string>();
		//Assert(false, std::format("String FOR COMPOENNT FIELD IS: {}", str == nullptr ? "NULL" : *str));
		static_cast<UIInputField*>(m_fields[0])->OverrideInput(*str);
	}
	else if (GetFieldInfo().IsCurrentType<Vec2>())
	{
		const Vec2* point = GetFieldInfo().TryGetValue<Vec2>();
		//Assert(false, std::format("REACHED COMPENZTN FIELD POINT: {}",point == nullptr ? "NULL" : point->ToString()));

		static_cast<UIInputField*>(m_fields[0])->OverrideInput(std::to_string(point->m_X));
		static_cast<UIInputField*>(m_fields[1])->OverrideInput(std::to_string(point->m_Y));

		//Assert(false, std::format("HAS PINT: {} Input field input is now: {} should be: {}", std::to_string(point!=nullptr), m_inputFields[0].GetInput(), std::to_string(point->m_X)));
		//Assert(false, std::format("Input field input is now: {} should be: {}", m_inputFields[1].GetInput(), std::to_string(point->m_Y)));
	}
	else if (GetFieldInfo().IsCurrentType<Vec2Int>())
	{
		const Vec2Int* point = GetFieldInfo().TryGetValue<Vec2Int>();
		static_cast<UIInputField*>(m_fields[0])->OverrideInput(std::to_string(point->m_X));
		static_cast<UIInputField*>(m_fields[1])->OverrideInput(std::to_string(point->m_Y));
	}
	else if (GetFieldInfo().IsCurrentType<bool>())
	{
		static_cast<UIToggleComponent*>(m_fields[0])->SetValue(*(GetFieldInfo().TryGetValue<bool>()));
	}
	else if (GetFieldInfo().IsCurrentType<Utils::Color>())
	{
		static_cast<UIColorPickerData*>(m_fields[0])->SetColor(*(GetFieldInfo().TryGetValue<Utils::Color>()));
	}
	else
	{
		LogError(std::format("Tried to set field to internal value for property: '{}' "
			"but could not find any actions for its type", GetFieldInfo().m_FieldName));
		return;
	}
}
void ComponentFieldUI::SetInternalWithInput()
{
	if (GetFieldInfo().IsReadonly()) return;

	/*LogError(std::format("Getting internal input and setting it. is point: {} field name: {} is point: {}", 
		m_fieldInfo.GetCurrentType().name(), m_fieldInfo.m_FieldName, std::to_string(m_fieldInfo.IsCurrentType<Vec2>())));*/

	if (GetFieldInfo().IsCurrentType<int>())
	{
		//LogError("REAHCED INT");
		//LogError(std::format("Setting internal value with: {}", std::to_string(m_inputFields[0].GetIntInput())));
		GetFieldInfo().TrySetValue<int>(static_cast<UIInputField*>(m_fields[0])->GetIntInput());
	}
	else if (GetFieldInfo().IsCurrentType<std::uint8_t>())
	{
		//LogError("REAHCED UNISGNED INT");
		//LogError(std::format("Setting internal value with: {}", std::to_string(m_inputFields[0].GetIntInput())));
		std::uint8_t convertedValue = static_cast<std::uint8_t>(std::abs(static_cast<UIInputField*>(m_fields[0])->GetIntInput()));
		GetFieldInfo().TrySetValue<std::uint8_t>(convertedValue);
	}
	else if (GetFieldInfo().IsCurrentType<float>())
	{
		//LogError("REAHCED FLOAT");
		GetFieldInfo().TrySetValue<float>(static_cast<UIInputField*>(m_fields[0])->GetFloatInput());
	}
	else if (GetFieldInfo().IsCurrentType<std::string>())
	{
		//LogError("REAHCED STRINGF");
		GetFieldInfo().TrySetValue<std::string>(static_cast<UIInputField*>(m_fields[0])->GetInput());
	}
	else if (GetFieldInfo().IsCurrentType<Vec2>())
	{
		//m_inputFields[0].GetFloatInput(), m_inputFields[1].GetFloatInput()
		//LogError("REAHCED POINT");
		GetFieldInfo().TrySetValue<Vec2>({ static_cast<UIInputField*>(m_fields[0])->GetFloatInput(), static_cast<UIInputField*>(m_fields[1])->GetFloatInput() });
		//Assert(false, std::format("REACHED COMPENZTN FIELD POINT: {}",point == nullptr ? "NULL" : point->ToString()));

		//Assert(false, std::format("HAS PINT: {} Input field input is now: {} should be: {}", std::to_string(point!=nullptr), m_inputFields[0].GetInput(), std::to_string(point->m_X)));
		//Assert(false, std::format("Input field input is now: {} should be: {}", m_inputFields[1].GetInput(), std::to_string(point->m_Y)));
	}
	else if (GetFieldInfo().IsCurrentType<Vec2Int>())
	{
		//m_inputFields[0].GetFloatInput(), m_inputFields[1].GetFloatInput()
		//LogError("REAHCED POINT");
		GetFieldInfo().TrySetValue<Vec2Int>({ static_cast<UIInputField*>(m_fields[0])->GetIntInput(), static_cast<UIInputField*>(m_fields[1])->GetIntInput() });
		//Assert(false, std::format("REACHED COMPENZTN FIELD POINT: {}",point == nullptr ? "NULL" : point->ToString()));

		//Assert(false, std::format("HAS PINT: {} Input field input is now: {} should be: {}", std::to_string(point!=nullptr), m_inputFields[0].GetInput(), std::to_string(point->m_X)));
		//Assert(false, std::format("Input field input is now: {} should be: {}", m_inputFields[1].GetInput(), std::to_string(point->m_Y)));
	}
	else if (GetFieldInfo().IsCurrentType<bool>())
	{
		GetFieldInfo().TrySetValue<bool>(static_cast<UIToggleComponent*>(m_fields[0])->IsToggled());
	}
	else if (GetFieldInfo().IsCurrentType<Utils::Color>())
	{
		GetFieldInfo().TrySetValue<Utils::Color>(static_cast<UIColorPickerData*>(m_fields[0])->GetColor());
	}
	else
	{
		//LogError("REAHCED ERROR");
		LogError(std::format("Tried to set internal value with input for property: '{}' "
			"but could not find any actions for its type", GetFieldInfo().m_FieldName));
	}
}

void ComponentFieldUI::Update()
{
	//NOTE: yes this might be bad to set to internal value every update, but
	//we do not know when the value internally can change, so it must be updated
	//every frame
	SetFieldToInternal();
	//TODO: update the values in the field
}

const ComponentField& ComponentFieldUI::GetFieldInfo() const
{
	if (!Assert(m_fieldInfo != nullptr, std::format("Tried to get field info from component field GUI but it is NULL")))
		throw std::invalid_argument("Invalid field info state");
	return *m_fieldInfo;
}

const ComponentUI& ComponentFieldUI::GetComponentGUISafe() const
{
	if (!Assert(m_componentGUI != nullptr, std::format("Tried to get component GUI "
		"from a field named : '{}'", GetFieldInfo().ToString())))
	{
		throw std::invalid_argument("Failed to retrieve entity gui");
	}
	return *m_componentGUI;
}
