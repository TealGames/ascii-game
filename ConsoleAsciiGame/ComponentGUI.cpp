#include "pch.hpp"
#include "ComponentGUI.hpp"
#include "raylib.h"
#include "Globals.hpp"
#include "RaylibUtils.hpp"
#include "Debug.hpp"
#include "Entity.hpp"
#include "EntityGUI.hpp"

constexpr static int MAX_PANEL_WIDTH = 100;
constexpr static int MAX_PANEL_HEIGHT = 100;

constexpr static float TITLE_FONT_SIZE = 10;
constexpr static float TITLE_FONT_SPACING = 2;

ComponentGUI::ComponentGUI(const Input::InputManager& inputManager, const EntityGUI& entityGUI, ComponentData* component)
	: m_inputManager(&inputManager), m_component(component), m_fieldGUIs(), m_entityGUI(&entityGUI)
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
		m_fieldGUIs.push_back(ComponentFieldGUI(GetInputManager(), *this, field));
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
	for (auto& field : m_fieldGUIs)
	{
		field.Update();
	}
}

std::string ComponentGUI::GetComponentName() const
{
	return m_component == nullptr ? "" : typeid(*m_component).name();
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
	DrawRectangle(currentPos.x, currentPos.y, renderInfo.m_RenderSize.m_X, MAX_PANEL_HEIGHT, DARKGRAY);

	DrawTextEx(GetGlobalFont(), componentName.c_str(), currentPos, TITLE_FONT_SIZE, TITLE_FONT_SPACING, WHITE);
	currentPos.y += MeasureTextEx(GetFontDefault(), componentName.c_str(), TITLE_FONT_SIZE, TITLE_FONT_SPACING).y;

	const int oneFieldHeight = (renderInfo.m_RenderSize.m_Y - (renderInfo.m_TopLeftPos.m_Y - currentPos.y)) / m_fieldGUIs.size();
	for (auto& fieldGUI : m_fieldGUIs)
	{
		currentPos.y += fieldGUI.Render(RenderInfo({ static_cast<int>(currentPos.x), static_cast<int>(currentPos.y) }, 
												   { renderInfo.m_RenderSize.m_X, oneFieldHeight })).m_Y;
	}

	return ScreenPosition{ MAX_PANEL_WIDTH, MAX_PANEL_HEIGHT };
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