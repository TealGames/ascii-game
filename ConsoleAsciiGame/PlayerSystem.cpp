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

	void PlayerSystem::SystemUpdate(Scene& scene, PlayerData& player,
		ECS::Entity& entity, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PlayerSystem::SystemUpdate");

#endif 
		InputData* input = entity.TryGetComponent<InputData>();
		if (!Utils::Assert(this, input != nullptr, std::format("Tried to move player from system update "
			"of PlayerSystem but it does not have a input component!"))) return;

		//Player moves faster on diagonals
		if (!input->HasInputChanged()) return;

		Vec2 dirDelta = GetVector(input->GetInputDelta());
		if (dirDelta == Vec2::ZERO) return;

		//TODO: if the player stops pressing a button we should remove that velocity
		dirDelta = dirDelta.GetNormalized();
		Vec2 inputVel = { dirDelta.m_X * player.GetMoveSpeed(), dirDelta.m_Y* player.GetInitialJumpSpeed()};

		//If we press up while not grounded, we stop that
		if (!player.GetIsGrounded())
		{
			if (inputVel.m_Y > 0) inputVel.m_Y = 0;

			Utils::Point2D rayOrigin = player.GetBodyMutableSafe().GetAABBPos({ 0, 0.5 });
			if (scene.GetPhysicsWorldMutable().Raycast()
		}

		player.GetBodyMutableSafe().SetVelocityDelta(inputVel);
	}
}
