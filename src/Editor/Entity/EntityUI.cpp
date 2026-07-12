#include "pch.hpp"
#include "Editor/Entity/EntityUI.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/UI/UIInteractionManager.hpp"
#include "Editor/EditorStyles.hpp"
#include "Core/Input/InputManager.hpp"
#include "Core/UIElementTemplates.hpp"
#include "ECS/Component/Types/UI/UITransformComponent.hpp"
#include "ECS/Component/Types/UI/UIToggleComponent.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "ECS/Component/Types/UI/UILayoutComponent.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"

namespace Engine::Editor::UI
{
	constexpr static float TITLE_FONT_SIZE = 20;
	constexpr float ACTIVE_TOGGLE_WIDTH = 0.1;
	constexpr float TOGGLE_NAME_SPACE_WIDTH = 0.05;
	const NormalizedVec2 TOP_LEFT_POS_NORMALIZED = { 0.8, 1 };

	EntityUI::EntityUI(const Input::InputManager& manager, UI::PopupUIManager& popupManager, Assets::AssetManager& assetManager)
		: m_inputManager(&manager), m_entity(nullptr), m_componentUIs(),
		m_entityNameText(nullptr),
		m_activeToggle(nullptr), m_guiLayout(nullptr), m_entityHeader(), m_layoutParent(nullptr),
		m_popupManager(&popupManager), m_assetManager(&assetManager)
	{

	}

	EntityUI::~EntityUI()
	{
		//Assert(false, std::format("Entity GUI for:{} destroyed", m_entity!=nullptr? m_entity->GetName() : "NULL"));
	}

	void EntityUI::Init(ECS::EntityData& parent)
	{
		m_layoutParent = &parent;
	}

	void EntityUI::CreateLayout()
	{
		auto [guiLayoutEntity, guiLayoutTransform] = m_layoutParent->CreateChildUI("EntityUILayout");
		m_guiLayout = &(guiLayoutEntity->AddComponent(UI::UILayoutComponent(UI::LayoutType::Vertical, UI::SizingType::ShrinkOnly)));
		guiLayoutTransform->SetLocalBoundsTLBR(TOP_LEFT_POS_NORMALIZED, UI::UI_RECT_BOTTOM_RIGHT);

		auto [headerEntity, headerTransform] = guiLayoutEntity->CreateChildUI("EntityHeader");
		m_entityHeader = &(headerEntity->AddComponent(UI::UIPanelComponent(Styles::EDITOR_PRIMARY_COLOR)));
		//m_entityHeader = &(headerEntity->AddComponent(UIPanel(RED)));
		headerTransform->SetLocalSize({ 1, 0.03 });

		ECS::EntityData* toggleEntity = nullptr;
		UI::UITransformComponent* toggleTransform = nullptr;
		std::tie(toggleEntity, toggleTransform, m_activeToggle) = UI::Templates::CreateCheckboxTemplate(*headerEntity, "EntityActiveToggle");
		toggleTransform->SetLocalBoundsTLBR(UI::UI_RECT_TOP_LEFT, { ACTIVE_TOGGLE_WIDTH, 0 });
		m_activeToggle->m_OnValueSet.AddListener([this](bool isChecked)-> void
			{
				m_entity->TrySetEntityActive(isChecked);
			});

		auto [nameTextEntity, nameTextTransform] = headerEntity->CreateChildUI("EntityNameText");
		m_entityNameText = &(nameTextEntity->AddComponent(UI::UITextComponent("", Styles::GetTextStyleFactorSize(UI::TextAlignment::CenterLeft))));
		nameTextTransform->SetLocalBoundsTLBR({ ACTIVE_TOGGLE_WIDTH + TOGGLE_NAME_SPACE_WIDTH, 1 }, UI::UI_RECT_BOTTOM_RIGHT);
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
	void EntityUI::SetEntity(ECS::EntityData& entity)
	{
		if (m_guiLayout == nullptr) CreateLayout();

		m_entity = &entity;
		m_guiLayout->GetEntityMutable().TrySetEntityActive(true);
		m_activeToggle->SetValue(m_entity->IsEntityActive());
		m_entityNameText->SetText(std::format("{}{}", m_entity->m_Name, m_entity->IsGlobal() ? "(G)" : ""));

		//TODO: make sure to find a way to add/retrieve all components to then add here
		//Assert(false, std::format("When adding all comps, entity: {} has:{}", entity.m_Name, std::to_string(entity.GetAllComponentsMutable().size())));
		auto& components = entity.GetAllComponentsMutable();
		m_componentUIs.reserve(components.size());

		for (size_t i = 0; i < components.size(); i++)
		{
			if (m_componentUIs.size() <= i)
			{
				m_componentUIs.emplace_back(*m_inputManager, *m_popupManager, *m_assetManager, *this, *m_guiLayout);
			}
			m_componentUIs[i].SetComponent(*components[i]);
		}
		//LogWarning(std::format("Setentity gui layout to:{}", m_guiLayout->GetEntity().TryGetComponent<UITransformComponent>()->GetSize().ToString()));
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
	const ECS::EntityData& EntityUI::GetEntity() const
	{
		ENGINE_ASSERT(m_entity != nullptr, "Tried to get entity from entity GUI it is in an invalid state");
		return *m_entity;
	}
}

