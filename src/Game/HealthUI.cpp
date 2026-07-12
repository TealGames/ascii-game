#include "pch.hpp"
#include "Game/HealthUI.hpp"
#include "Game/GameState.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "ECS/Component/Types/UI/UITransformComponent.hpp"
#include "ECS/Component/Types/UI/UILayoutComponent.hpp"
#include "Utils/Debug.hpp"
#include "Core/UI/UIHierarchy.hpp"
#include "StaticGlobals.hpp"

using namespace Engine;

static const NormalizedVec2 HEALTH_START_POS_RELATIVE = { 0.05, 0.95 };
static const NormalizedVec2 HEALTH_SIZE = { 0.03, 0.03 };
static constexpr ColHDR4 RED_ORANGE_COLOR = { 215, 71, 9, 255 };
static constexpr float HEALTH_FONT_SIZE = 24;

HealthUI::HealthUI() : m_health() {}

void HealthUI::Init(UI::UIHierarchy& hierarchy, GameState& state)
{
	m_health.reserve(GameState::MAX_HEALTH);
	auto [healthRootEntity, healthRootTransform] = hierarchy.CreateAtRoot(UI::TOP_LAYER, "HealthRoot");
	UI::UILayoutComponent& healthRootLayout = healthRootEntity->AddComponent(UI::UILayoutComponent(UI::LayoutType::Horizontal, UI::SizingType::ShrinkOnly, { 0.05, 0 }));
	healthRootTransform->SetLocalTopLeftPos(HEALTH_START_POS_RELATIVE);

	Rendering::ScreenFontProperties fontSettings = Rendering::ScreenFontProperties();/*Rendering::ScreenFontProperties(HEALTH_FONT_SIZE, GLOBAL_FONT_CHAR_SPACING.m_X,
		StaticReferenceGlobals::GetDefaultRaylibFont()); */
	//const VisualData healthVisualData = VisualData({ {TextChar(RED_ORANGE_COLOR, '@')}}, {0, 0}, fontSettings, VisualData::DEFAULT_PIVOT);

	//NormalizedPosition topLeftPos = {};
	for (size_t i = 0; i < GameState::MAX_HEALTH; i++)
	{
		auto [healthUIEntity, healthUITransform] = healthRootEntity->CreateChildUI("Health" + std::to_string(i));

		//healthUIEntity->AddComponent<EntityRendererData>(EntityRendererData{ healthVisualData, RenderLayerType::UI });
		UI::UITextComponent& healthText = healthUIEntity->AddComponent(UI::UITextComponent("@", fontSettings, RED_ORANGE_COLOR));
		healthUITransform->SetLocalSize(HEALTH_SIZE);

		healthUIEntity->TrySetEntityActive(i < state.GetHealth());
		m_health.push_back(healthUIEntity);
	}

	state.m_OnHealthChanged.AddListener([this](std::uint8_t oldHealth, std::uint8_t newHealth)-> void
		{
			const std::uint8_t delta = newHealth - oldHealth;
			if (delta == 0) return;

			//If we are increasing we want to make sure we move up one index to the first new health
			//otherwise if ew decrease we start with the corresponding old health index
			size_t currentIndex = delta > 0 ? oldHealth : oldHealth - 1;

			//If we are increasing, we want to move one up similar to start health so the first condition
			//does not result to false otherwise for decreasing it is the corresponding index
			const size_t targetIndex = delta > 0 ? newHealth : newHealth - 1;

			while (currentIndex != targetIndex)
			{
				if (!Assert(currentIndex >= 0 && currentIndex < m_health.size(), "Tried to update health UI from {} -> {} "
					"but current index:{} is out of bounds of health UI:[0, {})", std::to_string(oldHealth), std::to_string(newHealth),
					std::to_string(currentIndex), std::to_string(m_health.size())))
					return;

				m_health[currentIndex]->TrySetEntityActive(delta > 0);

				delta > 0 ? currentIndex++ : currentIndex--;
			}
		});
}

