#include "pch.hpp"
#include "ComponentGUI.hpp"
#include "raylib.h"
#include "Globals.hpp"
#include "RaylibUtils.hpp"
#include "Debug.hpp"
#include "Entity.hpp"
#include "EntityGUI.hpp"
#include "GUISelectorManager.hpp"
#include "EditorStyles.hpp"

constexpr static float HEADER_PANEL_HEIGHT = 0.03;
constexpr static float DROPDOWN_WIDTH = 0.2;

constexpr static float TITLE_FONT_SIZE = 10;
constexpr static float TITLE_FONT_SPACING = 2;

constexpr static float ONE_FIELD_MAX_ENTITY_SPACE = 0.05;

//If true, will divide fields based how many we have to render, otherwise will try give them as much space as possible
//static constexpr bool DIVIDE_FIELDS_BY_AMOUNT = false;

ComponentGUI::ComponentGUI(const Input::InputManager& inputManager, PopupGUIManager& popupManager, const EntityGUI& entityGUI, ComponentData& component)
	: m_inputManager(&inputManager), m_component(&component), m_fieldGUIs(), m_entityGUI(&entityGUI), 
	m_dropdownCheckbox(false, GUIStyle(EditorStyles::EDITOR_SECONDARY_COLOR, TextGUIStyle())),
	m_componentNameText(GetComponentName(), EditorStyles::GetTextStyleFactorSize(TextAlignment::CenterLeft)), 
	m_guiContainer(), m_fieldLayout(LayoutType::Vertical, SizingType::ExpandAndShrink, {0, 0.02}, EditorStyles::EDITOR_BACKGROUND_COLOR),
	m_nameHeader(EditorStyles::EDITOR_BACKGROUND_COLOR)
{
	/*Assert(false, std::format("Created compiennt gui for comp: {} with field val: {}", GetComponentName(), 
		std::get<Vec2*>(component->GetFieldsMutable()[0].m_Value)->ToString()));*/

	/*if (component->GetEntitySafe().m_Name=="player") Assert(false, std::format("Created compiennt gui for comp: {} with field val: {}", GetComponentName(),
		std::get<Vec2*>(component->GetFieldsMutable()[0].m_Value)->ToString())); */

	//component->InitFields();

	/*Assert(false, std::format("Init fields of comp: {} for entity: {} with; {}",
			GetComponentName(), GetEntityGUISafe().GetEntity().m_Name, 
			Utils::ToStringIterable<std::vector<ComponentField>, ComponentField>(component->GetFields())));*/

	m_nameHeader.SetSize({ 1, 1 });
	m_nameHeader.SetFixed(false, true);
	m_nameHeader.PushChild(&m_dropdownCheckbox);
	m_nameHeader.PushChild(&m_componentNameText);
	m_dropdownCheckbox.SetBounds(NormalizedPosition::TOP_LEFT, { DROPDOWN_WIDTH, 0});
	m_componentNameText.SetBounds({ DROPDOWN_WIDTH, 1}, NormalizedPosition::BOTTOM_RIGHT);

	m_guiContainer.PushChild(&m_nameHeader);
	m_guiContainer.SetSize({1, HEADER_PANEL_HEIGHT });

	auto& fields = component.GetFieldsMutable();
	m_fieldGUIs.reserve(fields.size());

	//size_t i = 0;
	float totalHeightNorm = 0;
	for (auto& field : fields)
	{
		m_fieldGUIs.emplace_back(GetInputManager(), popupManager, *this, field);
		LogError(std::format("Added component field: {}", m_fieldGUIs.back().GetTreeGUI()->ToStringBase()));
		//m_fieldGUIs.back().GetTreeGUI()->SetSize({ 1, float(1)/m_fieldGUIs.size()});
		m_fieldLayout.AddLayoutElement(m_fieldGUIs.back().GetTreeGUI());
		totalHeightNorm += m_fieldGUIs.back().GetTreeGUI()->GetSize().GetY();

		LogWarning(std::format("Adding field element parent:{}", m_fieldGUIs.back().GetTreeGUI()->ToStringBase()));
		//i++;
		//LogWarning(std::format("created field gui from compoennt:{}", Utils::ToStringPointerAddress(&m_fieldGUIs.back())));
	}
	//Assert(false, std::format("Poop"));

	m_dropdownCheckbox.SetValueSetAction([this, totalHeightNorm](const bool isChecked) -> void
		{
			if (isChecked)
			{
				//Assert(false, std::format("CHECK"));
				//LogWarning(std::format("Creating tree to compoennt gui:{}", m_fieldGUIs.back().GetTreeGUI()->ToStringRecursive("")));

				//m_guiContainer.SetSize(NormalizedPosition( 1, HEADER_PANEL_HEIGHT+ ONE_FIELD_MAX_ENTITY_SPACE * totalHeightNorm));
				m_guiContainer.SetSize(NormalizedPosition( 1, HEADER_PANEL_HEIGHT+ ONE_FIELD_MAX_ENTITY_SPACE * m_fieldGUIs.size()));

				m_fieldLayout.SetSize({1, 1- m_nameHeader.GetSize().GetY() });
				m_fieldLayout.SetTopLeftPos(NormalizedPosition::TOP_LEFT- Vec2(0, m_nameHeader.GetSize().GetY()));
				m_guiContainer.PushChild(&m_fieldLayout);

				//Assert(false, std::format("Component tree:{}", m_guiContainer.ToStringRecursive("")));
			}
			else
			{
				m_guiContainer.TryPopChildAt(1);
				//m_nameHeader.SetSize({1, 1});
				m_guiContainer.SetSize({ 1, HEADER_PANEL_HEIGHT });
			}
		});


	/*Assert(false, std::format("Created compiennt gui for comp: {} with field val: {}", GetComponentName(),
		std::get<Vec2*>(m_fieldGUIs[0].GetFieldInfo().m_Value)->ToString()));*/

	//Assert(false, std::format("Created compiennt gui for comp: {} with fields: {}", GetComponentName(), component->GetFields()[0].m_FieldName));
}
ComponentGUI::~ComponentGUI()
{
	//LogError("COMPOENNT GUI destroyed");
}

//void ComponentGUI::Init()
//{
//
//}

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
	//m_dropdownCheckbox.Update();
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
		fieldNames.emplace_back(field.GetFieldInfo().m_FieldName);
	}
	return fieldNames;
}

GUIElement* ComponentGUI::GetTreeGUI()
{
	return &m_guiContainer;
}
/*
ScreenPosition ComponentGUI::Render(const RenderInfo& renderInfo)
{
	//Assert(false, std::format("REDNER field: {}", m_fieldGUIs[0].GetFieldInfo().m_FieldName));
	//std::string componentName = GetComponentName();

	Vector2 currentPos = RaylibUtils::ToRaylibVector(renderInfo.m_TopLeftPos);

	//The first rectangle drawn is for the header ONLY
	//float componentHeight = MeasureTextEx(GetGlobalFont(), componentName.c_str(), TITLE_FONT_SIZE, TITLE_FONT_SPACING).y;
	const ScreenPosition componentNameTextSize = m_componentNameText.CalculateSize(renderInfo);
	float componentHeight = componentNameTextSize.m_Y;
	DrawRectangle(currentPos.x, currentPos.y, renderInfo.m_RenderSize.m_X, componentHeight, EntityEditorGUI::EDITOR_BACKGROUND_COLOR);
	
	ScreenPosition checkboxArea= m_dropdownCheckbox.Render(RenderInfo(ScreenPosition(currentPos.x, currentPos.y), ScreenPosition(TITLE_FONT_SIZE, TITLE_FONT_SIZE)));
	const Vec2 remainingTitleSpace = Vec2(renderInfo.m_RenderSize.m_X - checkboxArea.m_X, TITLE_FONT_SIZE);
	//const float componentNameFont = RaylibUtils::GetMaxFontSizeForSpace(GetGlobalFont(), componentName.c_str(), remainingTitleSpace, TITLE_FONT_SPACING);
	//Assert(false, std::format("max font for area: {} is: {}", remainingTitleSpace.ToString(), std::to_string(componentNameFont)));
	
	//DrawTextEx(GetGlobalFont(), componentName.c_str(), {currentPos.x+ checkboxArea.m_X, currentPos.y}, componentNameFont, TITLE_FONT_SPACING, EntityEditorGUI::EDITOR_TEXT_COLOR);

	m_componentNameText.Render(RenderInfo(ScreenPosition(currentPos.x + checkboxArea.m_X, currentPos.y), 
		ScreenPosition(remainingTitleSpace.m_X, remainingTitleSpace.m_Y)));
	
	//currentPos.y += MeasureTextEx(GetFontDefault(), componentName.c_str(), TITLE_FONT_SIZE, TITLE_FONT_SPACING).y;
	currentPos.y += componentNameTextSize.m_Y;
	if (!m_dropdownCheckbox.IsToggled())
	{
		return ScreenPosition(MAX_PANEL_WIDTH, componentHeight);
	}

	const Vector2 expandedComponentStartPos = currentPos;
	int fieldHeight = 0;
	if (DIVIDE_FIELDS_BY_AMOUNT) 
		fieldHeight = (renderInfo.m_RenderSize.m_Y - (currentPos.y - renderInfo.m_TopLeftPos.m_Y)) / m_fieldGUIs.size();

	ScreenPosition fieldSize = {};
	std::vector<Event<void>> renderActions = {};
	for (auto& fieldGUI : m_fieldGUIs)
	{
		//We subtract from current pos since as we go down y value increases
		if (!DIVIDE_FIELDS_BY_AMOUNT) fieldHeight = renderInfo.m_RenderSize.m_Y - (currentPos.y- renderInfo.m_TopLeftPos.m_Y);
		renderActions.emplace_back();
		fieldSize = fieldGUI.SetupRender(RenderInfo({ static_cast<int>(currentPos.x), static_cast<int>(currentPos.y) },
												{ renderInfo.m_RenderSize.m_X, fieldHeight }), renderActions.back());
		currentPos.y += fieldSize.m_Y;
		componentHeight += fieldSize.m_Y;
	}

	//Here we draw the expanded part. We had to get the full area first before we knew how much we could draw for the background
	const float expandedComponentHeight = currentPos.y - expandedComponentStartPos.y;
	DrawRectangle(expandedComponentStartPos.x, expandedComponentStartPos.y, renderInfo.m_RenderSize.m_X, expandedComponentHeight, EntityEditorGUI::EDITOR_BACKGROUND_COLOR);
	for (auto& renderAction : renderActions)
	{
		renderAction.Invoke();
		//Assert(false, std::format("Invoking action: {}", std::to_string(renderAction.GetListeners().size())));
	}
	//if (m_fieldGUIs.size() == 3) Assert(false, std::format("Poop"));

	//Assert(false, std::format("height: {} size used: {}", std::to_string(fieldHeight), fieldSize.ToString()));
	return ScreenPosition(MAX_PANEL_WIDTH, componentHeight);
}
*/

const EntityGUI& ComponentGUI::GetEntityGUISafe() const
{
	if (!Assert(this, m_entityGUI != nullptr, std::format("Tried to get entity GUI from cmponent: '{}' for entity: '{}'",
		GetComponentName(), m_component->GetEntitySafe().GetName())))
	{
		throw std::invalid_argument("Failed to retrieve entity gui");
	}
	return *m_entityGUI;
}