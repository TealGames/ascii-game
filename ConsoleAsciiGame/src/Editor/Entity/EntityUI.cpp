#include "pch.hpp"
#include "Editor/Entity/EntityUI.hpp"
#include "Utils/RaylibUtils.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/UI/UIInteractionManager.hpp"
#include "Editor/EditorStyles.hpp"
#include "Core/Input/InputManager.hpp"

constexpr static float TITLE_FONT_SIZE = 20;
constexpr float ACTIVE_TOGGLE_SPACE = 0.15;
const NormalizedPosition TOP_LEFT_POS_NORMALIZED = { 0.8, 1 };

EntityUI::EntityUI(const Input::InputManager& manager, PopupUIManager& popupManager, const AssetManagement::AssetManager& assetManager, EntityData& parent)
	: m_inputManager(&manager), m_entity(nullptr), m_componentUIs(), 
	m_entityNameText(nullptr),
	m_activeToggle(nullptr), m_guiLayout(nullptr), m_entityHeader(),
	m_popupManager(&popupManager), m_assetManager(&assetManager)
{
	auto [guiLayoutEntity, guiLayoutTransform] = parent.CreateChildUI("EntityUILayout");
	m_guiLayout = &(guiLayoutEntity->AddComponent(UILayout(LayoutType::Vertical, SizingType::ShrinkOnly)));
	guiLayoutTransform->SetBounds(TOP_LEFT_POS_NORMALIZED, NormalizedPosition::BOTTOM_RIGHT);

	auto[headerEntity, headerTransform] = guiLayoutEntity->CreateChildUI("EntityHeader");
	m_entityHeader = &(headerEntity->AddComponent(UIPanel(EditorStyles::EDITOR_PRIMARY_COLOR)));
	headerTransform->SetSize({ 1, 0.03 });

	auto [nameTextEntity, nameTextTransform] = headerEntity->CreateChildUI("EntityNameText");
	m_entityNameText = &(headerEntity->AddComponent(UITextComponent("", EditorStyles::GetTextStyleFactorSize(TextAlignment::CenterLeft))));
	nameTextTransform->SetBounds({ ACTIVE_TOGGLE_SPACE, 1 }, NormalizedPosition::BOTTOM_RIGHT);

	auto [toggleEntity, toggleTransform] = guiLayoutEntity->CreateChildUI("ActiveToggle");
	m_activeToggle= &(headerEntity->AddComponent(UIToggleComponent(false, EditorStyles::GetToggleStyle())));
	toggleTransform->SetBounds(NormalizedPosition::TOP_LEFT, { ACTIVE_TOGGLE_SPACE, 0 });
	m_activeToggle->SetValueSetAction([this](bool isChecked)-> void 
		{
			m_entity->TrySetEntityActive(isChecked);
		});
}

EntityUI::~EntityUI()
{
	//Assert(false, std::format("Entity GUI for:{} destroyed", m_entity!=nullptr? m_entity->GetName() : "NULL"));
}

void EntityUI::Update()
{
	//m_activeToggle.Update();
	//LogError(std::format("Has valid selector:{}", std::to_string(m_activeToggle.GetSelectorManager().SelectedSelectableThisFrame())));
	for (auto& componentUI : m_componentUIs)
	{
		componentUI.Update();
	}

	//We need to make sure we update the toggle to match a value if active was 
	//set internally and not via UI
	//Note: side effect is bool var "active" will be set to value again as before due
	//to callback occuring when value is set
	if (m_entity->IsEntityActive() != m_activeToggle->IsToggled())
		m_activeToggle->SetValue(m_entity->IsEntityActive());
}
void EntityUI::SetEntity(EntityData& entity)
{
	m_entity = &entity;
	m_guiLayout->GetEntityMutable().TrySetEntityActive(true);
	m_activeToggle->SetValue(m_entity->IsEntityActive());
	m_entityNameText->SetText(std::format("{}{}", m_entity->m_Name, m_entity->IsGlobal() ? "(G)" : ""));

	//TODO: make sure to find a way to add/retrieve all components to then add here
	//Assert(false, std::format("When adding all comps, entity: {} has:{}", entity.m_Name, std::to_string(entity.GetAllComponentsMutable().size())));
	auto& components = entity.GetAllComponentsMutable();
	m_componentUIs.reserve(components.size());

	for (size_t i=0; i<components.size(); i++)
	{
		if (m_componentUIs.size() <= i)
		{
			m_componentUIs.emplace_back(ComponentUI(*m_inputManager, *m_popupManager, *m_assetManager, *this, *m_guiLayout));
		}
		m_componentUIs[i].SetComponent(*components[i]);
		//i++;
		//m_guiLayout.AddLayoutElement(m_componentGUIs.back().GetTreeGUI());
	}
}
void EntityUI::ClearEntity()
{
	if (!HasEntity()) return;
	m_entity = nullptr;
	m_guiLayout->GetEntityMutable().DeactivateEntity();
}
bool EntityUI::HasEntity() const
{
	return m_entity != nullptr;
}
const EntityData& EntityUI::GetEntity() const
{
	if (!Assert(this, m_entity != nullptr, std::format("Tried to get entity from entity GUI it is in an invalid state")))
		throw std::invalid_argument("Invalid entity gui entity state");

	return *m_entity;
}
