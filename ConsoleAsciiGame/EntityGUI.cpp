#include "pch.hpp"
#include "EntityGUI.hpp"
#include "RaylibUtils.hpp"
#include "HelperFunctions.hpp"
#include "GUISelectorManager.hpp"
#include "EditorStyles.hpp"

constexpr static float TITLE_FONT_SIZE = 20;
constexpr float ACTIVE_TOGGLE_SPACE = 0.15;

EntityGUI::EntityGUI(const Input::InputManager& manager, PopupGUIManager& popupManager, const AssetManagement::AssetManager& assetManager, ECS::Entity& entity)
	: m_inputManager(&manager), m_entity(&entity), m_componentGUIs(), 
	m_entityNameText(std::format("{}{}", m_entity->GetName(), m_entity->IsGlobal()? "(G)" : ""), EditorStyles::GetTextStyleFactorSize(TextAlignment::CenterLeft)),
	m_activeToggle(m_entity->m_Active, EditorStyles::GetToggleStyle()),
	m_guiLayout(LayoutType::Vertical, SizingType::ShrinkOnly), m_entityHeader(EditorStyles::EDITOR_PRIMARY_COLOR)
{

	m_activeToggle.SetValueSetAction([this](bool isChecked)-> void 
		{
			m_entity->m_Active = isChecked;
		});

	m_entityHeader.SetSize({1, 0.03 });
	m_entityHeader.PushChild(&m_activeToggle);
	m_activeToggle.SetBounds(NormalizedPosition::TOP_LEFT, { ACTIVE_TOGGLE_SPACE, 0});

	m_entityHeader.PushChild(&m_entityNameText);
	m_entityNameText.SetBounds({ ACTIVE_TOGGLE_SPACE, 1 }, NormalizedPosition::BOTTOM_RIGHT);

	m_guiLayout.AddLayoutElement(&m_entityHeader);

	//TODO: make sure to find a way to add/retrieve all components to then add here
	//Assert(false, std::format("When adding all comps, entity: {} has:{}", entity.m_Name, std::to_string(entity.GetAllComponentsMutable().size())));
	auto components = entity.GetAllComponentsMutable();
	m_componentGUIs.reserve(components.size());

	for (auto& comp : components)
	{
		if (comp == nullptr) continue;
		m_componentGUIs.emplace_back(*m_inputManager, popupManager, assetManager, *this, *comp);

		m_guiLayout.AddLayoutElement(m_componentGUIs.back().GetTreeGUI());
		//i++;
		//m_guiLayout.AddLayoutElement(m_componentGUIs.back().GetTreeGUI());
	}
	//Assert(false, std::format("Added"));
	
	//Assert(false, "POOP");
	/*Assert(false, std::format("For entity: {} found fields: {}", entity.m_Name, 
		Utils::ToStringIterable<std::vector<ComponentField>, ComponentField>(entity.m_Transform.GetFields())));*/

	//Assert(false, std::format("Size: {}", std::to_string(m_componentGUIs.size())));
	
	//if (entity.m_Name== "player") Assert(false, std::format("First field VALUE IS: {}", m_componentGUIs[0].GetFields()[0].GetFieldInfo().TryGetValue<Utils::Point2D>()->ToString()));

	/*
	for (const auto& field : m_componentGUIs[0].GetFieldNames())
	{
		Assert(false, std::format("Found field; {}", field));
	}

	for (const auto& componentGUI : m_componentGUIs)
	{
		Assert(false, std::format("Found comp: {} fields:{}", componentGUI.GetComponentName(),
			Utils::ToStringIterable<std::vector<std::string>, std::string>(componentGUI.GetFieldNames())));
	}
	*/
}

EntityGUI::~EntityGUI()
{
	//Assert(false, std::format("Entity GUI for:{} destroyed", m_entity!=nullptr? m_entity->GetName() : "NULL"));
}

void EntityGUI::SetComponentsToStored()
{
	for (auto& component : m_componentGUIs)
	{
		component.SetFieldsToStored();
	}
}

void EntityGUI::Update()
{
	//m_activeToggle.Update();
	//LogError(std::format("Has valid selector:{}", std::to_string(m_activeToggle.GetSelectorManager().SelectedSelectableThisFrame())));
	for (auto& component : m_componentGUIs)
	{
		component.Update();
	}

	//We need to make sure we update the toggle to match a value if active was 
	//set internally and not via UI
	//Note: side effect is bool var "active" will be set to value again as before due
	//to callback occuring when value is set
	if (m_entity->m_Active != m_activeToggle.IsToggled())
		m_activeToggle.SetValue(m_entity->m_Active);
}

GUIElement* EntityGUI::GetTreeGUI()
{
	return &m_guiLayout;
}

/*
ScreenPosition EntityGUI::Render(const RenderInfo& renderInfo)
{
	Vector2 currentPos = RaylibUtils::ToRaylibVector(renderInfo.m_TopLeftPos);
	//Vector2 textSize = MeasureTextEx(GetGlobalFont(), m_entity.GetName().c_str(), TITLE_FONT_SIZE, GLOBAL_CHAR_SPACING.m_X);
	Vector2 textSize = RaylibUtils::ToRaylibVector(m_entityNameText.CalculateSize(renderInfo));
	//Assert(false, std::format("Text size: {}", RaylibUtils::ToString(textSize)));

	DrawRectangle(currentPos.x, currentPos.y, renderInfo.m_RenderSize.m_X, textSize.y, EntityEditorGUI::EDITOR_PRIMARY_COLOR);
	ScreenPosition toggleSize= m_activeToggle.Render(RenderInfo(renderInfo.m_TopLeftPos, ScreenPosition(textSize.y, textSize.y)));
	//DrawTextEx(GetGlobalFont(), m_entity.GetName().c_str(), currentPos, TITLE_FONT_SIZE, GLOBAL_CHAR_SPACING.m_X, EntityEditorGUI::EDITOR_TEXT_COLOR);
	m_entityNameText.Render(RenderInfo(renderInfo.m_TopLeftPos+ ScreenPosition(toggleSize.m_X, 0), {static_cast<int>(textSize.x), static_cast<int>(textSize.y)}));

	currentPos.y += textSize.y;
	//currentPos.y += MeasureTextEx(GetGlobalFont(), m_entity.m_Name.c_str(), GLOBAL_FONT_SIZE, GLOBAL_CHAR_SPACING.m_X).y;

	//Assert(false, std::format("Entity start pos: {}", RaylibUtils::ToString(startPos)));
	//ScreenPosition entityGUISize = {renderInfo.m_RenderSize.m_X, static_cast<int>(renderInfo.m_RenderSize.m_Y / m_componentGUIs.size())};
	ScreenPosition componentSpaceLeft = {};
	ScreenPosition componentSize = {};

	for (auto& componentGUI : m_componentGUIs)
	{
		//Assert(false, std::format("Found comp: {} fields:{}", componentGUI.GetComponentName(), 
		//Utils::ToStringIterable<std::vector<std::string>, std::string>(componentGUI.GetFieldNames())));
		componentSpaceLeft = ScreenPosition(renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y- (currentPos.y - renderInfo.m_TopLeftPos.m_Y));
		//LogError(std::format("Space left:{}", componentSpaceLeft.ToString()));
		componentSize= componentGUI.Render(RenderInfo({ static_cast<int>(currentPos.x), static_cast<int>(currentPos.y)}, componentSpaceLeft));
		currentPos.y += componentSize.m_Y;
	}
	//Assert(false, std::format("END OF THE ROAD"));
	return renderInfo.m_RenderSize;
}
*/

const ECS::Entity& EntityGUI::GetEntity() const
{
	if (!Assert(this, m_entity != nullptr, std::format("Tried to get entity from entity GUI it is in an invalid state")))
		throw std::invalid_argument("Invalid entity gui entity state");

	return *m_entity;
}
