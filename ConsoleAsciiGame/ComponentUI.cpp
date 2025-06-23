#include "pch.hpp"
#include "ComponentUI.hpp"
#include "raylib.h"
#include "Globals.hpp"
#include "RaylibUtils.hpp"
#include "Debug.hpp"
#include "Entity.hpp"
#include "EntityUI.hpp"
#include "UIInteractionManager.hpp"
#include "EditorStyles.hpp"
#include "AssetManager.hpp"
#include "TextureAsset.hpp"

constexpr static float HEADER_PANEL_HEIGHT = 0.03;
constexpr static float DROPDOWN_WIDTH = 0.2;

constexpr static float TITLE_FONT_SIZE = 10;
constexpr static float TITLE_FONT_SPACING = 2;

constexpr static float ONE_FIELD_MAX_ENTITY_SPACE = 0.05;

//If true, will divide fields based how many we have to render, otherwise will try give them as much space as possible
//static constexpr bool DIVIDE_FIELDS_BY_AMOUNT = false;

ComponentUI::ComponentUI(const Input::InputManager& inputManager, PopupUIManager& popupManager, 
	const AssetManagement::AssetManager& m_assetManager, const EntityUI& entityGUI, UILayout& parent)
	: m_inputManager(&inputManager), m_popupManager(&popupManager), m_component(nullptr), m_fieldGUIs(), m_entityGUI(&entityGUI),
	m_dropdownDefaultTexture(m_assetManager.TryGetTypeAsset<TextureAsset>("dropdown_icon_default")), 
	m_dropdownToggledTexture(m_assetManager.TryGetTypeAsset<TextureAsset>("dropdown_icon_toggled")),
	m_dropdownCheckbox(nullptr), m_componentNameText(nullptr),  m_container(nullptr), m_fieldLayout(nullptr), m_nameHeader(nullptr)
{
	EntityData* guiContainerEntity = nullptr;
	std::tie(guiContainerEntity, m_container) = parent.CreateLayoutElement("ComponentContainer");
	m_container->SetSize({ 1, HEADER_PANEL_HEIGHT });

	auto [nameHeaderEntity, nameHeaderTransform] = guiContainerEntity->CreateChildUI("NameHeader");
	m_nameHeader = &(nameHeaderEntity->AddComponent(UIPanel(EditorStyles::EDITOR_BACKGROUND_COLOR)));
	nameHeaderTransform->SetSize({ 1, 1 });
	nameHeaderTransform->SetFixed(false, true);

	auto [dropdownEntity, dropdownTransform] = nameHeaderEntity->CreateChildUI("DropdownCheckbox");
	m_dropdownCheckbox = &(dropdownEntity->AddComponent(UIToggleComponent(false, EditorStyles::GetToggleStyle(), nullptr, m_dropdownDefaultTexture)));
	dropdownTransform->SetBounds(NormalizedPosition::TOP_LEFT, { DROPDOWN_WIDTH, 0 });
	m_dropdownCheckbox->SetValueSetAction([this](const bool isChecked) -> void
		{
			if (isChecked)
			{
				//Assert(false, std::format("CHECK"));
				//LogWarning(std::format("Creating tree to compoennt gui:{}", m_fieldGUIs.back().GetTreeGUI()->ToStringRecursive("")));

				//m_guiContainer.SetSize(NormalizedPosition( 1, HEADER_PANEL_HEIGHT+ ONE_FIELD_MAX_ENTITY_SPACE * totalHeightNorm));
				m_container->SetSize(NormalizedPosition(1, HEADER_PANEL_HEIGHT + ONE_FIELD_MAX_ENTITY_SPACE * m_fieldGUIs.size()));
				NormalizedPosition nameHeaderSize = m_nameHeader->GetEntityMutable().TryGetComponentMutable<UITransformData>()->GetSize();
				UITransformData& layoutTransform = *(m_fieldLayout->GetEntityMutable().TryGetComponentMutable<UITransformData>());

				layoutTransform.SetSize({ 1, 1 - nameHeaderSize.GetY() });
				layoutTransform.SetTopLeftPos(NormalizedPosition::TOP_LEFT - Vec2(0, nameHeaderSize.GetY()));
				

				if (m_dropdownToggledTexture != nullptr) m_dropdownCheckbox->SetOverlayTexture(*m_dropdownToggledTexture);
				//Assert(false, std::format("Component tree:{}", m_guiContainer.ToStringRecursive("")));
			}
			else
			{
				//m_nameHeader.SetSize({1, 1});
				m_container->SetSize({ 1, HEADER_PANEL_HEIGHT });
				if (m_dropdownDefaultTexture != nullptr) m_dropdownCheckbox->SetOverlayTexture(*m_dropdownDefaultTexture);
			}
			m_fieldLayout->GetEntityMutable().TrySetEntityActive(isChecked);
		});

	auto[nameTextEntity, nameTextTransform] = nameHeaderEntity->CreateChildUI("NameText");
	m_componentNameText = &(nameTextEntity->AddComponent(UITextComponent("", EditorStyles::GetTextStyleFactorSize(TextAlignment::CenterLeft))));
	nameTextTransform->SetBounds({ DROPDOWN_WIDTH, 1 }, NormalizedPosition::BOTTOM_RIGHT);

	auto [layoutEntity, layoutTransform] = guiContainerEntity->CreateChildUI("Layout");
	//TODO: add colored panel with color: EditorStyles::EDITOR_BACKGROUND_COLOR to feld layout background
	m_fieldLayout = &(nameTextEntity->AddComponent(UILayout(LayoutType::Vertical, SizingType::ExpandAndShrink, NormalizedPosition{ 0, 0.02 })));
	/*Assert(false, std::format("Created compiennt gui for comp: {} with field val: {}", GetComponentName(),
		std::get<Vec2*>(m_fieldGUIs[0].GetFieldInfo().m_Value)->ToString()));*/

	//Assert(false, std::format("Created compiennt gui for comp: {} with fields: {}", GetComponentName(), component->GetFields()[0].m_FieldName));
}
ComponentUI::~ComponentUI()
{
	//LogError("COMPOENNT GUI destroyed");
}

//void ComponentGUI::Init()
//{
//
//}

void ComponentUI::SetComponent(Component& component)
{
	m_component = &component;

	auto& fields = component.GetFieldsMutable();
	m_fieldGUIs.reserve(fields.size());

	//size_t i = 0;
	//float totalHeightNorm = 0;
	for (size_t i=0; i< fields.size(); i++)
	{
		if (m_fieldGUIs.size() <= i) m_fieldGUIs.emplace_back(GetInputManager(), *m_popupManager, *this, *m_fieldLayout);
		m_fieldGUIs[i].SetField(fields[i]);
		//LogError(std::format("Added component field: {}", m_fieldGUIs.back().GetTreeGUI()->ToStringBase()));
		//m_fieldGUIs.back().GetTreeGUI()->SetSize({ 1, float(1)/m_fieldGUIs.size()});
		//totalHeightNorm += m_fieldGUIs.back().GetTreeGUI()->GetSize().GetY();

		//LogWarning(std::format("Adding field element parent:{}", m_fieldGUIs.back().GetTreeGUI()->ToStringBase()));
		//i++;
		//LogWarning(std::format("created field gui from compoennt:{}", Utils::ToStringPointerAddress(&m_fieldGUIs.back())));
	}
}

const Input::InputManager& ComponentUI::GetInputManager() const
{
	if (!Assert(this, m_inputManager != nullptr, 
		std::format("Tried to get input manager for component GUI but it is NULL")))
	{
		throw std::invalid_argument("Invalid input manager state");
	}

	return *m_inputManager;
}

void ComponentUI::Update()
{
	//m_dropdownCheckbox.Update();
	for (auto& field : m_fieldGUIs)
	{
		field.Update();
	}
}

std::string ComponentUI::GetComponentName() const
{
	std::string formattedName= m_component == nullptr ? "" : Utils::FormatTypeName(typeid(*m_component).name());
	//Assert(false, std::format("formatted name: {}", formattedName));
	return formattedName;
}

const std::vector<ComponentFieldUI>& ComponentUI::GetFields() const
{
	return m_fieldGUIs;
}

std::vector<std::string> ComponentUI::GetFieldNames() const
{
	//return {std::format("Size of: {}", m_fieldGUIs[0].GetFieldInfo().m_FieldName)};

	std::vector<std::string> fieldNames = {};
	for (const auto& field : m_fieldGUIs)
	{
		fieldNames.emplace_back(field.GetFieldInfo().m_FieldName);
	}
	return fieldNames;
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

const EntityUI& ComponentUI::GetEntityGUISafe() const
{
	if (!Assert(this, m_entityGUI != nullptr, std::format("Tried to get entity GUI from cmponent: '{}' for entity: '{}'",
		GetComponentName(), m_component->GetEntity().m_Name)))
	{
		throw std::invalid_argument("Failed to retrieve entity gui");
	}
	return *m_entityGUI;
}