#include "pch.hpp"
#include "PlayerSystem.hpp"
#include "PhysicsBodyData.hpp"
#include "CameraData.hpp"
#include "Vec2.hpp"
#include "raylib.h"
#include "PositionConversions.hpp"
#include "Scene.hpp"
#include "EntityData.hpp"

#ifdef ENABLE_PROFILER
#include "ProfilerTimer.hpp"
#endif 

namespace ECS
{
	static constexpr bool CHEATS_ENABLED_DEFAULT = false;
	//If true, will jump immediately when landing and the up key is pressed
	static constexpr bool DO_AUTO_JUMP = true;

	PlayerSystem::PlayerSystem(Input::InputManager& input) : 
		m_inputManager(input), m_cheatsEnabled(CHEATS_ENABLED_DEFAULT), m_lastFrameGrounded(false)
	{
		GlobalComponentInfo::AddComponentInfo(typeid(PlayerData), ComponentInfo(CreateRequiredComponentFunction<PhysicsBodyData>()));
	}

	void PlayerSystem::SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PlayerSystem::SystemUpdate");
#endif 

		if (deltaTime <= 0) return;
		
		scene.OperateOnComponents<PlayerData>(
			[this, &scene, &deltaTime, &mainCamera](PlayerData& player)-> void
			{
#ifdef ALLOW_PLAYER_CHEATS
				if (m_cheatsEnabled && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
				{
					/*CameraData* camera = scene.TryGetMainCameraMutable();
					if (!Assert(this, camera != nullptr, std::format("Tried to get camera to convert screen "
						"to world point for mouse position cheat but it is null"))) return;*/

					Vector2 mousePos = GetMousePosition();
					WorldPosition worldPos = Conversions::ScreenToWorldPosition(mainCamera, { static_cast<int>(mousePos.x), static_cast<int>(mousePos.y) });
					player.GetEntityMutable().GetTransformMutable().SetLocalPos(worldPos);
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
				if (player.GetLastFrameInput() != Vec2Int::ZERO &&
					player.GetBodyMutableSafe().GetVelocity() == Vec2::ZERO)
				{
					player.SetFrameInput(Vec2Int::ZERO);
				}
				

				//TODO: Player moves faster on diagonals
				if (!player.HasInputChanged()) return;

				Vec2Int inputDelta = player.GetInputDelta();
				Vec2 dirDelta = Vec2(inputDelta.m_X, inputDelta.m_Y);
				if (dirDelta == Vec2::ZERO) return;

				//Auto jumping occurs when we are grounded but the input is held (meaning we might not have been changed since last frame
				//but to make sure we immediately jump back up, we ignore it and consider it anways) 
				//TODO: maybe this explicit auto jump is not good and we should instead have input queueing
				if (DO_AUTO_JUMP && player.GetIsGrounded() && !m_lastFrameGrounded &&
					moveCompound->TryGetDirectionAction(Input::InputDirection::Up)->IsPressed())
				{
					dirDelta.m_Y = 1;
				}

				//TODO: normalized should be handled in input retrieving
				dirDelta = dirDelta.GetNormalized();

				//If we press up while not grounded, we stop that
				//if (!player.GetIsGrounded() && inputVel.m_X>0) inputVel.m_Y = 0;

				player.GetBodyMutableSafe().SetVelocityDelta({ dirDelta.m_X * player.GetMoveSpeed(), 0});

				//If we are grounded and press up, only for that moment do we then apply the initial jump speed
				if (player.GetIsGrounded() && dirDelta.m_Y > 0)
					player.GetBodyMutableSafe().SetVelocityDelta({ 0, dirDelta.m_Y * player.GetInitialJumpSpeed() });

				//If we stop pressing up while still going up, we cancel further movement up
				const float playerYVel = player.GetBodyMutableSafe().GetVelocity().m_Y;
				if (!player.GetIsGrounded() && playerYVel > 0 && dirDelta.m_Y < 0)
					player.GetBodyMutableSafe().SetVelocityYDelta(-playerYVel);

				m_lastFrameGrounded = player.GetIsGrounded();
			});
	}

	void PlayerSystem::SetCheatStatus(const bool& enableCheats)
	{
		m_cheatsEnabled = enableCheats;
	}
}
