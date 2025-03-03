#include "pch.hpp"
#include "ComponentGUI.hpp"
#include "raylib.h"
#include "Globals.hpp"
#include "RaylibUtils.hpp"
#include "Debug.hpp"
#include "Entity.hpp"
#include "EntityGUI.hpp"
#include "GUISelectorManager.hpp"
#include "EntityEditorGUI.hpp"

constexpr static int MAX_PANEL_WIDTH = 100;
constexpr static int MAX_PANEL_HEIGHT = 100;

constexpr static float TITLE_FONT_SIZE = 10;
constexpr static float TITLE_FONT_SPACING = 2;

//If true, will divide fields based how many we have to render, otherwise will try give them as much space as possible
static constexpr bool DIVIDE_FIELDS_BY_AMOUNT = false;

ComponentGUI::ComponentGUI(const Input::InputManager& inputManager, GUISelectorManager& selector, const EntityGUI& entityGUI, ComponentData* component)
	: m_inputManager(&inputManager), m_component(component), m_fieldGUIs(), m_entityGUI(&entityGUI), 
	m_dropdownCheckbox(selector, false, GUISettings({}, EntityEditorGUI::EDITOR_SECONDARY_COLOR, TextGUISettings()))
{
	if (component == nullptr) return;

	/*Assert(false, std::format("Created compiennt gui for comp: {} with field val: {}", GetComponentName(), 
		std::get<Utils::Point2D*>(component->GetFieldsMutable()[0].m_Value)->ToString()));*/

	/*if (component->GetEntitySafe().m_Name=="player") Assert(false, std::format("Created compiennt gui for comp: {} with field val: {}", GetComponentName(),
		std::get<Utils::Point2D*>(component->GetFieldsMutable()[0].m_Value)->ToString())); */

	component->InitFields();

	/*Assert(false, std::format("Init fields of comp: {} for entity: {} with; {}",
			GetComponentName(), GetEntityGUISafe().GetEntity().m_Name, 
			Utils::ToStringIterable<std::vector<ComponentField>, ComponentField>(component->GetFields())));*/

	for (auto& field : component->GetFieldsMutable())
	{
		m_fieldGUIs.push_back(ComponentFieldGUI(GetInputManager(), selector, *this, field));
	}

	/*Assert(false, std::format("Created compiennt gui for comp: {} with field val: {}", GetComponentName(),
		std::get<Utils::Point2D*>(m_fieldGUIs[0].GetFieldInfo().m_Value)->ToString()));*/

	//Assert(false, std::format("Created compiennt gui for comp: {} with fields: {}", GetComponentName(), component->GetFields()[0].m_FieldName));
}
ComponentGUI::~ComponentGUI()
{
	//LogError("COMPOENNT GUI destroyed");
}

const Input::InputManager& ComponentGUI::GetInputManager() const
{
	if (!Assert(this, m_inputManager != nullptr, 
		std::format("Tried to get input manager for component GUI but it is NULL")))
	{
		throw std::invalid_argument("Invalid input manager state");
	}

	return *m_inputManager;
}

void ComponentGUI::SetFieldsToStored()
{
	for (auto& field : m_fieldGUIs)
	{
		field.SetFieldToInternal();
	}
}

void ComponentGUI::Update()
{
	m_dropdownCheckbox.Update();
	for (auto& field : m_fieldGUIs)
	{
		field.Update();
	}
}

std::string ComponentGUI::GetComponentName() const
{
	std::string formattedName= m_component == nullptr ? "" : Utils::FormatTypeName(typeid(*m_component).name());
	//Assert(false, std::format("formatted name: {}", formattedName));
	return formattedName;
}

const std::vector<ComponentFieldGUI>& ComponentGUI::GetFields() const
{
	return m_fieldGUIs;
}

std::vector<std::string> ComponentGUI::GetFieldNames() const
{
	//return {std::format("Size of: {}", m_fieldGUIs[0].GetFieldInfo().m_FieldName)};

	std::vector<std::string> fieldNames = {};
	for (const auto& field : m_fieldGUIs)
	{
		fieldNames.push_back(field.GetFieldInfo().m_FieldName);
	}
	return fieldNames;
}

ScreenPosition ComponentGUI::Render(const RenderInfo& renderInfo)
{
	//Assert(false, std::format("REDNER field: {}", m_fieldGUIs[0].GetFieldInfo().m_FieldName));
	std::string componentName = GetComponentName();

	Vector2 currentPos = RaylibUtils::ToRaylibVector(renderInfo.m_TopLeftPos);
	//Assert(false, std::format("Drawing rect at pos: {}", RaylibUtils::ToString(currentPos)));
	float componentHeight = MAX_PANEL_HEIGHT;

	if (!m_dropdownCheckbox.IsChecked())
	{
		componentHeight = MeasureTextEx(GetGlobalFont(), componentName.c_str(), TITLE_FONT_SIZE, TITLE_FONT_SPACING).y;
	}
	DrawRectangle(currentPos.x, currentPos.y, renderInfo.m_RenderSize.m_X, componentHeight, EntityEditorGUI::EDITOR_BACKGROUND_COLOR);

	ScreenPosition checkboxArea= m_dropdownCheckbox.Render(RenderInfo(ScreenPosition(currentPos.x, currentPos.y), ScreenPosition(TITLE_FONT_SIZE, TITLE_FONT_SIZE)));
	Utils::Point2D remainingTitleSpace = Utils::Point2D(renderInfo.m_RenderSize.m_X - checkboxArea.m_X, TITLE_FONT_SIZE);
	const float componentNameFont = RaylibUtils::GetMaxFontSizeForSpace(GetGlobalFont(), componentName.c_str(), remainingTitleSpace, TITLE_FONT_SPACING);
	//Assert(false, std::format("max font for area: {} is: {}", remainingTitleSpace.ToString(), std::to_string(componentNameFont)));
	DrawTextEx(GetGlobalFont(), componentName.c_str(), {currentPos.x+ checkboxArea.m_X, currentPos.y}, componentNameFont, TITLE_FONT_SPACING, EntityEditorGUI::EDITOR_TEXT_COLOR);
	
	currentPos.y += MeasureTextEx(GetFontDefault(), componentName.c_str(), TITLE_FONT_SIZE, TITLE_FONT_SPACING).y;
	if (!m_dropdownCheckbox.IsChecked())
	{
		return ScreenPosition(MAX_PANEL_WIDTH, componentHeight);
	}

	int fieldHeight = 0;
	if (DIVIDE_FIELDS_BY_AMOUNT) 
		fieldHeight = (renderInfo.m_RenderSize.m_Y - (currentPos.y - renderInfo.m_TopLeftPos.m_Y)) / m_fieldGUIs.size();

	ScreenPosition fieldSize = {};
	for (auto& fieldGUI : m_fieldGUIs)
	{
		//We subtract from current pos since as we go down y value increases
		if (!DIVIDE_FIELDS_BY_AMOUNT) fieldHeight = renderInfo.m_RenderSize.m_Y - (currentPos.y- renderInfo.m_TopLeftPos.m_Y);
		fieldSize = fieldGUI.Render(RenderInfo({ static_cast<int>(currentPos.x), static_cast<int>(currentPos.y) },
												{ renderInfo.m_RenderSize.m_X, fieldHeight }));
		/*if (m_fieldGUIs.size() == 3)
		{
			LogError(std::format("found field of name: {} with size: {} one hegiht: {} total space: {}",
				fieldGUI.GetFieldInfo().m_FieldName, std::to_string(fieldSize.m_Y), std::to_string(oneFieldHeight), std::to_string(renderInfo.m_RenderSize.m_Y)));
		}*/
		currentPos.y += fieldSize.m_Y;
	}
	//if (m_fieldGUIs.size() == 3) Assert(false, std::format("Poop"));

	return ScreenPosition(MAX_PANEL_WIDTH, componentHeight);
}

const EntityGUI& ComponentGUI::GetEntityGUISafe() const
{
	if (!Assert(this, m_entityGUI != nullptr, std::format("Tried to get entity GUI from cmponent: '{}' for entity: '{}'",
		GetComponentName(), m_component->GetEntitySafe().m_Name)))
	{
		throw std::invalid_argument("Failed to retrieve entity gui");
	}
	return *m_entityGUI;
}