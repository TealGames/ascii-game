#include "pch.hpp"
#include "PlayerSystem.hpp"
#include "HelperFunctions.hpp"
#include "PhysicsBodyData.hpp"
#include "CameraData.hpp"
#include "InputData.hpp"
#include "Vec2.hpp"
#include "ProfilerTimer.hpp"
#include "raylib.h"
#include "PositionConversions.hpp"

namespace ECS
{
	static constexpr bool CHEATS_ENABLED_DEFAULT = false;

	PlayerSystem::PlayerSystem() : m_cheatsEnabled(CHEATS_ENABLED_DEFAULT) {}

	void PlayerSystem::SystemUpdate(Scene& scene, PlayerData& player,
		ECS::Entity& entity, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PlayerSystem::SystemUpdate");

#endif 
		if (m_cheatsEnabled && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
		{
			CameraData* camera = scene.TryGetMainCameraData();
			if (!Assert(this, camera != nullptr, std::format("Tried to get camera to convert screen "
				"to world point for mouse position cheat but it is null"))) return;

			Vector2 mousePos = GetMousePosition();
			WorldPosition worldPos = Conversions::ScreenToWorldPosition(*camera, { static_cast<int>(mousePos.x), static_cast<int>(mousePos.y)});
			player.GetEntitySafeMutable().m_Transform.SetPos(worldPos);
		}

		bool isGroundedNow = player.GetIsGrounded();
		if (isGroundedNow != m_isGroudnedLastFrame)
		{
			if (groundedTimes >= 2)
			{
				LogError(this, std::format("GROUNDED CHANGED TO: {} distance:{}", std::to_string(isGroundedNow),
					std::to_string(player.GetVerticalDistanceToGround())), true, true);
				//throw new std::invalid_argument("POOP");
			}
			else
			{
				groundedTimes++;
			}
		}
		m_isGroudnedLastFrame = isGroundedNow;

		InputData* input = entity.TryGetComponent<InputData>();
		if (!Assert(this, input != nullptr, std::format("Tried to move player from system update "
			"of PlayerSystem but it does not have a input component!"))) return;

		//TODO: Player moves faster on diagonals
		if (!input->HasInputChanged()) return;

		Vec2 dirDelta = GetVector(input->GetInputDelta());
		if (dirDelta == Vec2::ZERO) return;

		//TODO: normalized should be handled in input retrieving
		dirDelta = dirDelta.GetNormalized();
		Vec2 inputVel = { dirDelta.m_X * player.GetMoveSpeed(), dirDelta.m_Y* player.GetInitialJumpSpeed()};

		//If we press up while not grounded, we stop that
		if (!player.GetIsGrounded())
		{
			if (inputVel.m_Y > 0) inputVel.m_Y = 0;

			Utils::Point2D rayOrigin = player.GetBodyMutableSafe().GetAABBWorldPos({ 0, 0.5 });
			//TODO: add raycast to ground to add auto jump when holding up before ground hit
			//if (scene.GetPhysicsWorldMutable().Raycast()
		}

		player.GetBodyMutableSafe().SetVelocityDelta(inputVel);
	}

	void PlayerSystem::SetCheatStatus(const bool& enableCheats)
	{
		m_cheatsEnabled = enableCheats;
	}
}
