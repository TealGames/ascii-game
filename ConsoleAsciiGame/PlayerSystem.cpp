#include "pch.hpp"
#include "PlayerSystem.hpp"
#include "HelperFunctions.hpp"
#include "PhysicsBodyData.hpp"
#include "CameraData.hpp"
#include "InputData.hpp"
#include "Vec2.hpp"
#include "raylib.h"
#include "PositionConversions.hpp"

#ifdef ENABLE_PROFILER
#include "ProfilerTimer.hpp"
#endif 

namespace ECS
{
	static constexpr bool CHEATS_ENABLED_DEFAULT = false;

	PlayerSystem::PlayerSystem(Input::InputManager& input) : 
		m_inputManager(input), m_cheatsEnabled(CHEATS_ENABLED_DEFAULT), m_lastFrameGrounded(false)
	{}

	void PlayerSystem::SystemUpdate(Scene& scene, PlayerData& player,
		ECS::Entity& entity, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PlayerSystem::SystemUpdate");
#endif 

#ifdef ALLOW_PLAYER_CHEATS
		if (m_cheatsEnabled && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
		{
			CameraData* camera = scene.TryGetMainCameraData();
			if (!Assert(this, camera != nullptr, std::format("Tried to get camera to convert screen "
				"to world point for mouse position cheat but it is null"))) return;

			Vector2 mousePos = GetMousePosition();
			WorldPosition worldPos = Conversions::ScreenToWorldPosition(*camera, { static_cast<int>(mousePos.x), static_cast<int>(mousePos.y)});
			player.GetEntitySafeMutable().m_Transform.SetPos(worldPos);
		}
#endif

		const Input::InputProfile* inputProfile = m_inputManager.TryGetProfile(MAIN_INPUT_PROFILE_NAME);
		if (!Assert(this, inputProfile != nullptr, std::format("Tried to move player in PlayerSystem "
			"but the input profile: '{}' was not found", MAIN_INPUT_PROFILE_NAME)))
			return;

		const Input::CompoundInput* moveCompound = inputProfile->TryGetCompoundInputAction(MAIN_INPUT_PROFILE_MOVE_ACTION);
		if (!Assert(this, moveCompound != nullptr, std::format("Tried to move player in PlayerSystem "
			"but the move compound: '{}' was not found in input profile: {}",
			MAIN_INPUT_PROFILE_MOVE_ACTION, MAIN_INPUT_PROFILE_NAME)))
			return;

		player.SetFrameInput(moveCompound->GetCompoundInputDown());

		//This is a special case where if the physics system has overriden 
		//velocity with zero EVEN WHEN WE HAD INPUT (meaning that input was canceled)
		//we need to update the last frame input so the input system does not think
		//the last input was successful (otherwise we could get bad input delta)
		if (player.GetLastFrameInput() != Utils::Point2DInt::ZERO &&
			player.GetBodyMutableSafe().GetVelocity() == Vec2::ZERO)
		{
			player.SetFrameInput(Utils::Point2DInt::ZERO);
		}

		//TODO: Player moves faster on diagonals
		if (!player.HasInputChanged()) return;

		Vec2 dirDelta = GetVector(player.GetInputDelta());
		if (dirDelta == Vec2::ZERO) return;

		//Auto jumping occurs when we are grounded but the input is held (meaning we might not have been changed since last frame
		//but to make sure we immediately jump back up, we ignore it and consider it anways) 
		//TODO: maybe this explicit auto jump is not good and we should instead have input queueing
		if (player.GetIsGrounded() && !m_lastFrameGrounded && 
			moveCompound->TryGetDirectionAction(Direction::Up)->IsPressed())
		{
			dirDelta.m_Y = 1;
		}

		//TODO: normalized should be handled in input retrieving
		dirDelta = dirDelta.GetNormalized();
		Vec2 inputVel = { dirDelta.m_X * player.GetMoveSpeed(), dirDelta.m_Y* player.GetInitialJumpSpeed()};

		//If we press up while not grounded, we stop that
		if (!player.GetIsGrounded() && inputVel.m_Y > 0) inputVel.m_Y = 0;

		player.GetBodyMutableSafe().SetVelocityDelta(inputVel);
		m_lastFrameGrounded = player.GetIsGrounded();
	}

	void PlayerSystem::SetCheatStatus(const bool& enableCheats)
	{
		m_cheatsEnabled = enableCheats;
	}
}
