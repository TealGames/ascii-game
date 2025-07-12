#include "pch.hpp"
#include "ECS/Systems/Types/World/PlayerSystem.hpp"
#include "ECS/Component/Types/World/PhysicsBodyData.hpp"
#include "ECS/Component/Types/World/CameraData.hpp"
#include "Utils/Data/Vec2.hpp"
#include "raylib.h"
#include "Core/PositionConversions.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 

namespace ECS
{
	static constexpr bool CHEATS_ENABLED_DEFAULT = false;
	//If true, will jump immediately when landing and the up key is pressed
	static constexpr bool DO_AUTO_JUMP = true;

	PlayerSystem::PlayerSystem(Input::InputManager& input) : 
		m_inputManager(input), m_cheatsEnabled(CHEATS_ENABLED_DEFAULT), m_lastFrameGrounded(false)
	{
		GlobalComponentInfo::AddComponentInfo(typeid(PlayerData), 
			ComponentInfo(CreateComponentTypes<PhysicsBodyData>(), CreateRequiredComponentFunction(PhysicsBodyData()),
			[](EntityData& entity)-> void
			{
				PlayerData& player = *(entity.TryGetComponentMutable<PlayerData>());
				if (player.m_body != nullptr) player.m_body = entity.TryGetComponentMutable<PhysicsBodyData>();
				//fieldComponent.m_background = entity.TryGetComponentMutable<UIPanel>();
			}));
	}

	void PlayerSystem::SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PlayerSystem::SystemUpdate");
#endif 
		//LogError(std::format("Does scene have active player:{}", std::to_string(scene.TryGetEntity("player")->IsEntityActive())));
		if (deltaTime <= 0)
		{
			LogError(std::format("Regturning player due to dt:{}", deltaTime));
			return;
		}
		
		scene.OperateOnComponents<PlayerData>(
			[this, &scene, &deltaTime, &mainCamera](PlayerData& player)-> void
			{
#ifdef ALLOW_PLAYER_CHEATS
				if (m_cheatsEnabled && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
				{
					/*CameraData* camera = scene.TryGetMainCameraMutable();
					if (!Assert(camera != nullptr, std::format("Tried to get camera to convert screen "
						"to world point for mouse position cheat but it is null"))) return;*/

					WorldPosition worldPos = Conversions::ScreenToWorldPosition(mainCamera, m_inputManager.GetMousePosition());
					player.GetEntityMutable().GetTransformMutable().SetLocalPos(worldPos);
				}
#endif

				//LogWarning(std::format("Player is at pos:{}", player.GetEntity().GetTransform().GetGlobalPos().ToString()));

				const Input::InputProfile* inputProfile = m_inputManager.TryGetProfile(MAIN_INPUT_PROFILE_NAME);
				if (!Assert(inputProfile != nullptr, std::format("Tried to move player in PlayerSystem "
					"but the input profile: '{}' was not found", MAIN_INPUT_PROFILE_NAME)))
					return;

				const Input::CompoundInput* moveCompound = inputProfile->TryGetCompoundInputAction(MAIN_INPUT_PROFILE_MOVE_ACTION);
				if (!Assert(moveCompound != nullptr, std::format("Tried to move player in PlayerSystem "
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
