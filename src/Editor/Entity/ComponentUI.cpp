#include "pch.hpp"
#include "Editor/Entity/ComponentUI.hpp"
#include "StaticGlobals.hpp"
#include "Utils/Debug.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Editor/Entity/EntityUI.hpp"
#include "Core/UI/UIInteractionManager.hpp"
#include "Editor/EditorStyles.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "Core/UIElementTemplates.hpp"
#include "ECS/Component/Types/UI/UIToggleComponent.hpp"
#include "ECS/Component/Types/UI/UITransformComponent.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"
#include "ECS/Component/Types/UI/UILayoutComponent.hpp"

namespace Engine::Editor::UI
{
	constexpr static float HEADER_PANEL_HEIGHT = 0.03;
	constexpr static float DROPDOWN_WIDTH = 0.1;

	constexpr static float TITLE_FONT_SIZE = 10;
	constexpr static float TITLE_FONT_SPACING = 2;
	constexpr static float TITLE_DROPDOWN_SPACING = 0.05;

	constexpr static float ONE_FIELD_MAX_ENTITY_SPACE = 0.05;

	//If true, will divide fields based how many we have to render, otherwise will try give them as much space as possible
	//static constexpr bool DIVIDE_FIELDS_BY_AMOUNT = false;

	ComponentUI::ComponentUI(const Input::InputManager& inputManager, UI::PopupUIManager& popupManager,
		Assets::AssetManager& m_assetManager, const EntityUI& entityGUI, UI::UILayoutComponent& parent)
		: m_inputManager(&inputManager), m_popupManager(&popupManager), m_component(nullptr), m_fieldGUIs(), m_entityGUI(&entityGUI),
		m_dropdownCheckbox(nullptr), m_componentNameText(nullptr), m_container(nullptr), m_fieldLayout(nullptr), m_nameHeader(nullptr)
	{
		ECS::EntityData* guiContainerEntity = nullptr;
		std::tie(guiContainerEntity, m_container) = parent.CreateLayoutElement("ComponentContainer");
		m_container->SetLocalSize({ 1, HEADER_PANEL_HEIGHT });

		auto [nameHeaderEntity, nameHeaderTransform] = guiContainerEntity->CreateChildUI("ComponentHeader");
		m_nameHeader = &(nameHeaderEntity->AddComponent(UI::UIPanelComponent(Styles::EDITOR_BACKGROUND_COLOR)));
		nameHeaderTransform->SetLocalSize({ 1, 1 });
		nameHeaderTransform->SetFixed(false, true);

		auto [nameTextEntity, nameTextTransform] = nameHeaderEntity->CreateChildUI("ComponentNameText");
		m_componentNameText = &(nameTextEntity->AddComponent(UI::UITextComponent("", Styles::GetTextStyleFactorSize(UI::TextAlignment::CenterLeft))));
		nameTextTransform->SetLocalBoundsTLBR({ DROPDOWN_WIDTH, 1 }, UI::UI_RECT_BOTTOM_RIGHT);

		auto [layoutEntity, layoutTransform] = guiContainerEntity->CreateChildUI("ComponentLayout");
		//TODO: add colored panel with color: EditorStyles::EDITOR_BACKGROUND_COLOR to feld layout background
		m_fieldLayout = &(layoutEntity->AddComponent(UI::UILayoutComponent(UI::LayoutType::Vertical, UI::SizingType::ExpandAndShrink, NormalizedVec2{ 0, 0.02 })));
		/*Assert(false, std::format("Created compiennt gui for comp: {} with field val: {}", GetComponentName(),
			std::get<Vec2*>(m_fieldGUIs[0].GetFieldInfo().m_Value)->ToString()));*/

		ECS::EntityData* dropdownEntity = nullptr;
		UI::UITransformComponent* dropdownTransform = nullptr;
		std::tie(dropdownEntity, dropdownTransform, m_dropdownCheckbox) = UI::Templates::CreateDropdownToggleTemplate(*nameHeaderEntity, "ComponentDropdownCheckbox");
		dropdownTransform->SetLocalBoundsTLBR(UI::UI_RECT_TOP_LEFT, { DROPDOWN_WIDTH, 0 });
		m_dropdownCheckbox->m_OnValueSet.AddListener([this, nameHeaderTransform, layoutEntity, layoutTransform](const bool isChecked) -> void
			{
				if (isChecked)
				{
					//Assert(false, std::format("CHECK"));
					//LogWarning(std::format("Creating tree to compoennt gui:{}", m_fieldGUIs.back().GetTreeGUI()->ToStringRecursive("")));

					//m_guiContainer.SetSize(NormalizedPosition( 1, HEADER_PANEL_HEIGHT+ ONE_FIELD_MAX_ENTITY_SPACE * totalHeightNorm));
					m_container->SetLocalSize(NormalizedVec2(1, HEADER_PANEL_HEIGHT + ONE_FIELD_MAX_ENTITY_SPACE * m_fieldGUIs.size()));
					NormalizedVec2 nameHeaderSize = nameHeaderTransform->GetLocalSize();

					layoutTransform->SetLocalSize({ 1, 1 - nameHeaderSize.GetY() });
					layoutTransform->SetLocalTopLeftPos(UI::UI_RECT_TOP_LEFT - Vec2(0, nameHeaderSize.GetY()));
					//Assert(false, std::format("Component tree:{}", m_guiContainer.ToStringRecursive("")));
				}
				else
				{
					//m_nameHeader.SetSize({1, 1});
					m_container->SetLocalSize({ 1, HEADER_PANEL_HEIGHT });
				}
				layoutEntity->TrySetEntityActive(isChecked);
			});
		//Assert(false, std::format("Created compiennt gui for comp: {} with fields: {}", GetComponentName(), component->GetFields()[0].m_FieldName));
	}
	ComponentUI::~ComponentUI()
	{
		//LogError("COMPOENNT GUI destroyed");
	}

	//votid ComponentGUI::Init()
	//{
	//
	//}

	void ComponentUI::SetComponent(ECS::Component& component)
	{
		m_component = &component;
		m_componentNameText->SetText(GetComponentName());

		auto& fields = component.GetFieldsMutable();
		m_fieldGUIs.reserve(fields.size());

		//size_t i = 0;
		//float totalHeightNorm = 0;
		for (size_t i = 0; i < fields.size(); i++)
		{
			if (m_fieldGUIs.size() <= i) m_fieldGUIs.emplace_back(GetInputManager(), *m_popupManager, *this, *m_fieldLayout);
			m_fieldGUIs[i].SetField(fields[i]);
			//LogError(std::format("Added component field: {}", m_fieldGUIs.back().GetTreeGUI()->ToStringBase()));
			//m_fieldGUIs.back().GetTreeGUI()->SetSize({ 1, float(1)/m_fieldGUIs.size()});
			//totalHeightNorm += m_fieldGUIs.back().GetTreeGUI()->GetSize().GetY();

			//LogWarning(std::format("Adding field element parent:{}", m_fieldGUIs.back().GetTreeGUI()->ToStringBase()));
			//i++;
			//LogWarning(std::format("created field gui from compoennt:{}", ::Utils::ToStringPointerAddress(&m_fieldGUIs.back())));
		}
		m_fieldLayout->GetEntityMutable().DeactivateEntity();

		//NormalizedPosition nameHeaderSize = m_componentNameText->GetEntity().TryGetComponent<UITransformComponent>()->GetSize();
		//UITransformComponent& layoutTransform = *(m_fieldLayout->GetEntityMutable().TryGetComponentMutable<UITransformComponent>());
		//layoutTransform.SetSize({ 1, 1 - nameHeaderSize.GetY() });
		//layoutTransform.SetTopLeftPos(NormalizedPosition::TOP_LEFT - Vec2(0, nameHeaderSize.GetY()));
		//m_fieldLayout->GetEntityMutable().DeactivateEntity();
	}

	const Input::InputManager& ComponentUI::GetInputManager() const
	{
		ENGINE_ASSERT(m_inputManager != nullptr, "Tried to get input manager for component GUI but it is NULL");
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
		std::string formattedName = m_component == nullptr ? "" : ECS::FormatComponentName(typeid(*m_component));
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

	const EntityUI& ComponentUI::GetEntityGUISafe() const
	{
		ENGINE_ASSERT(m_entityGUI != nullptr, "Tried to get entity GUI from cmponent: '{}' for entity: '{}'",
			GetComponentName(), m_component->GetEntity().m_Name);
		return *m_entityGUI;
	}
}
