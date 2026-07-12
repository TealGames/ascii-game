#include "pch.hpp"
#include "Game/GlobalCreator.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"
#include "StaticGlobals.hpp"
#include "ECS/Component/Types/World/LightSource2DComponent.hpp"
#include "ECS/Component/Types/World/EntityRenderer2DComponent.hpp"
#include "ECS/Component/Types/World/AnimatorComponent.hpp"
#include "ECS/Component/Types/World/CameraComponent.hpp"
#include "ECS/Component/Types/World/TriggerComponent.hpp"
#include "ECS/Component/Types/World/CollisionBoxComponent.hpp"
#include "ECS/Component/Types/World/SpriteAnimatorComponent.hpp"
#include "ECS/Component/Types/World/PhysicsBodyComponent.hpp"
#include "ECS/Component/Types/World/PlayerComponent.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "Core/Camera/CameraController.hpp"
#include "ECS/Component/Types/World/ParticleEmitterComponent.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/SpriteAnimationAsset.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

namespace Engine::Scenes::GlobalEntityCreator
{
	void OnGlobalsInit(GlobalEntityManager& globalsManager, Scenes::SceneManager& sceneManager, 
		Camera::CameraController& cameraController, Assets::AssetManager& assetManager)
	{
		//Rendering::WorldFontProperties fontSettings = Rendering::WorldFontProperties(Rendering::VisualData::DEFAULT_FONT_SIZE, GLOBAL_FONT_CHAR_SPACING.m_X, 
		//	StaticReferenceGlobals::GetDefaultRaylibFont());

		ECS::EntityData& playerEntity = globalsManager.CreateGlobalEntity("player", TransformComponent(Vec3{ 10, 5, 0 }));
		Physics::CollisionBoxComponent& playerCollider = playerEntity.AddComponent(Physics::CollisionBoxComponent(Vec2(2, 2), Vec2(0, 0)));
		Physics::PhysicsBodyComponent& playerRB = playerEntity.AddComponent(Physics::PhysicsBodyComponent(&playerCollider, 1, GRAVITY, 20));
		Player::PlayerComponent& PlayerComponent = playerEntity.AddComponent(Player::PlayerComponent(playerRB, 8, 20));

		//InputData& inputData = playerEntity.AddComponent<InputData>(InputData{});
		Lighting2D::LightSource2DComponent& lightSource = playerEntity.AddComponent(Lighting2D::LightSource2DComponent{ 8, Rendering::RenderLayerType::Background,
			ColorGradient(ColHDR4(243, 208, 67, 255), ColHDR4(228, 8, 10, 255)), std::uint8_t(254), 1.2f });

		/*playerEntity.AddComponent(Rendering::EntityRenderer2DComponent{
			Rendering::VisualData(std::vector<std::vector<TextChar>>{ {TextChar(COLOR_GRAY, 'H') }}, {0, 0}, 
				fontSettings, Rendering::VisualData::DEFAULT_PIVOT), RenderLayerType::Player });*/

		ECS::ComponentFieldReference lightRadiusref = ECS::ComponentFieldReference(&lightSource, "Radius");
		//Assert(false, std::format("Entity light radius: {}", lightRadiusref.m_Entity->ToString()));
		ECS::ComponentField& field = lightRadiusref.GetComponentFieldSafeMutable();
		//Assert(false, std::format("Light source ref field: {}", field.ToString()));

		playerEntity.AddComponent(Animation::AnimatorComponent(std::vector<Animation::AnimationPropertyVariant>{
			Animation::AnimationProperty<std::uint8_t>(lightRadiusref, {
			Animation::AnimationPropertyKeyframe<std::uint8_t>(std::uint8_t(8), 0),
			Animation::AnimationPropertyKeyframe<std::uint8_t>(std::uint8_t(1), 1) })}, 1, 1, true));

		Animation::SpriteAnimatorComponent& spriteAnimator= playerEntity.AddComponent(Animation::SpriteAnimatorComponent());
		Animation::SpriteAnimationAsset* testAnim = assetManager.TryGetTypeAssetFromPathMutable<Animation::SpriteAnimationAsset>("sprite_animations/test.sanim");
		//Assert(false, std::format("Adding anim:{}", testAnim->ToString()));
		spriteAnimator.AddAnimation(*testAnim);
		spriteAnimator.TryPlayAnimation(testAnim->GetAnimation().m_Name);

		//Note: the camera has to be moved back a little so objects at origin are still seen due to min near plane value= 0.1
		ECS::EntityData& mainCameraEntity = globalsManager.CreateGlobalEntity("MainCamera", TransformComponent(Vec3(0, 0, -0.1), Vec3::One(), Math::Quat::Identity()));
		Camera::CameraComponent& cameraData = mainCameraEntity.AddComponent(Camera::CameraComponent{ Camera::CameraSettings{SCREEN_ASPECT_RATIO, 10, nullptr} });
		cameraController.TryRegisterCamera(cameraData);

		ECS::EntityData& trigger = globalsManager.CreateGlobalEntity("Trigger", TransformComponent(Vec3{15, 0, 0}));
		Physics::CollisionBoxComponent& triggerCollider = trigger.AddComponent(Physics::CollisionBoxComponent(Vec2(5, 5), Vec2(0, 0)));
		World::TriggerComponent& TriggerComponent= trigger.AddComponent(World::TriggerComponent(&triggerCollider));
	}
}

