#pragma once
#include <vector>

namespace Engine::ECS { class EntityData; }
namespace Engine::Scenes { class GlobalEntityManager; }
namespace Engine::UI { class UIHierarchy; }

class GameState;
class HealthUI
{
private:
	std::vector<Engine::ECS::EntityData*> m_health;
public:

private:
public:
	HealthUI();

	void Init(Engine::UI::UIHierarchy& hierarchy, GameState& state);
};
