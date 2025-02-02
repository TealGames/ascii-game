#include "pch.hpp"
#include "PlayerSystem.hpp"
#include "HelperFunctions.hpp"
#include "PhysicsBodyData.hpp"
#include "InputData.hpp"
#include "Vec2.hpp"
#include "ProfilerTimer.hpp"

namespace ECS
{
	PlayerSystem::PlayerSystem() {}

	void PlayerSystem::SystemUpdate(Scene& scene, PlayerData& component,
		ECS::Entity& entity, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PlayerSystem::SystemUpdate");

#endif 
		PhysicsBodyData* body = entity.TryGetComponent<PhysicsBodyData>();
		if (!Utils::Assert(this, body != nullptr, std::format("Tried to move player from system update "
			"of PlayerSystem but it does not have a physicsBody!"))) return;

		InputData* input = entity.TryGetComponent<InputData>();
		if (!Utils::Assert(this, body != nullptr, std::format("Tried to move player from system update "
			"of PlayerSystem but it does not have a input component!"))) return;

		//Player moves faster on diagonals
		if (!input->HasInputChanged()) return;

		Vec2 dirDelta = GetVector(input->GetInputDelta());
		if (dirDelta == Vec2::ZERO) return;

		//TODO: if the player stops pressing a button we should remove that velocity
		dirDelta = dirDelta.GetNormalized();
		body->SetVelocityDelta(dirDelta * component.GetMoveSpeed());
	}
}
