#include "pch.hpp"
#include "Game/HealthUI.hpp"
#include "Game/GameState.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/World/EntityRendererData.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Core/UI/UIHierarchy.hpp"

static const NormalizedPosition HEALTH_START_POS_RELATIVE = { 0.05, 0.95 };
static const NormalizedPosition HEALTH_SIZE = { 0.05, 0.05 };

namespace Game
{
	namespace UI
	{
		HealthUI::HealthUI() : m_health() {}

		void HealthUI::Init(UIHierarchy& hierarchy, GameState& state)
		{
			m_health.reserve(GameState::MAX_HEALTH);
			auto [healthRootEntity, healthRootTransform] = hierarchy.CreateAtRoot(DEFAULT_LAYER, "HealthRoot");

			/*const VisualDataPreset visualPreset = { GetGlobalFont(), 4, VisualData::DEFAULT_CHAR_SPACING,
					CharAreaType::Predefined, VisualData::DEFAULT_PREDEFINED_CHAR_AREA, VisualData::DEFAULT_PIVOT };*/
			FontProperties fontSettings = FontProperties(VisualData::DEFAULT_FONT_SIZE, GLOBAL_CHAR_SPACING.m_X, GetGlobalFont());
			const VisualData healthVisualData = VisualData({ {TextChar(Color(215, 71, 9, 255), '@')}}, {0, 0}, fontSettings, VisualData::DEFAULT_PIVOT);

			NormalizedPosition topLeftPos = {};
			for (size_t i = 0; i < GameState::MAX_HEALTH; i++)
			{
				auto [healthUIEntity, healthUITransform] = healthRootEntity->CreateChildUI("Health" + std::to_string(i));
				topLeftPos = NormalizedPosition(HEALTH_START_POS_RELATIVE.GetPos().m_X +
					HEALTH_SIZE.GetX() * i, HEALTH_START_POS_RELATIVE.GetPos().m_Y);

				healthUIEntity->AddComponent<EntityRendererData>(EntityRendererData{ healthVisualData, RenderLayerType::UI });
				healthUIEntity->TrySetEntityActive(i < state.GetHealth());

				m_health.push_back(healthUIEntity);
				//LogError(std::format("Craeted health:{}", healthPosRelative.GetPos().ToString()));
			}
			//Assert(false, std::format("Created Health UI max:{} globals:{}", std::to_string(GameState::MAX_HEALTH), globalEntities.ToStringEntityData()));

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
						if (!Assert(this, currentIndex >= 0 && currentIndex < m_health.size(), std::format("Tried to update health UI from {} -> {} "
							"but current index:{} is out of bounds of health UI:[0, {})", std::to_string(oldHealth), std::to_string(newHealth),
							std::to_string(currentIndex), std::to_string(m_health.size()))))
							return;

						m_health[currentIndex]->TrySetEntityActive(delta > 0);

						delta > 0 ? currentIndex++ : currentIndex--;
					}
				});
		}
	}
}

