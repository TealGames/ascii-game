#include "pch.hpp"
#include "Game/GlobalCreator.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"
#include "StaticGlobals.hpp"
#include "ECS/Component/Types/World/LightSourceData.hpp"
#include "ECS/Component/Types/World/EntityRendererData.hpp"
#include "ECS/Component/Types/World/AnimatorData.hpp"
#include "ECS/Component/Types/World/CameraData.hpp"
#include "ECS/Component/Types/World/TriggerData.hpp"
#include "ECS/Component/Types/World/CollisionBoxData.hpp"
#include "ECS/Component/Types/World/SpriteAnimatorData.hpp"
#include "ECS/Component/Types/World/PhysicsBodyData.hpp"
#include "ECS/Component/Types/World/PlayerData.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "Core/Camera/CameraController.hpp"
#include "ECS/Component/Types/World/ParticleEmitterData.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/SpriteAnimationAsset.hpp"
#include "Utils/Data/ColorConstants.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "StaticReferenceGlobals.hpp"

namespace GlobalEntityCreator
{
	void CreateGlobals(GlobalEntityManager& globalsManager, SceneManagement::SceneManager& sceneManager, 
		CameraController& cameraController, AssetManagement::AssetManager& assetManager)
	{
		WorldFontProperties fontSettings = WorldFontProperties(VisualData::DEFAULT_FONT_SIZE, GLOBAL_FONT_CHAR_SPACING.m_X, 
			StaticReferenceGlobals::GetDefaultRaylibFont());

		EntityData& playerEntity = globalsManager.CreateGlobalEntity("player", TransformComponent(Vec3{ 10, 5, 0 }));
		CollisionBoxData& playerCollider = playerEntity.AddComponent<CollisionBoxData>(CollisionBoxData(Vec2(2, 2), Vec2(0, 0)));
		PhysicsBodyData& playerRB = playerEntity.AddComponent<PhysicsBodyData>(PhysicsBodyData(&playerCollider, 1, GRAVITY, 20));
		PlayerData& playerData = playerEntity.AddComponent<PlayerData>(PlayerData(playerRB, 8, 20));

		//InputData& inputData = playerEntity.AddComponent<InputData>(InputData{});
		LightSourceData& lightSource = playerEntity.AddComponent<LightSourceData>(LightSourceData{ 8, RenderLayerType::Background,
			ColorGradient(Color(243, 208, 67, 255), Color(228, 8, 10, 255)), std::uint8_t(254), 1.2f });

		playerEntity.AddComponent<EntityRendererData>(EntityRendererData{
			VisualData(std::vector<std::vector<TextChar>>{ {TextChar(COLOR_GRAY, 'H') }}, {0, 0}, 
				fontSettings, VisualData::DEFAULT_PIVOT), RenderLayerType::Player });

		ComponentFieldReference lightRadiusref = ComponentFieldReference(&lightSource, "Radius");
		//Assert(false, std::format("Entity light radius: {}", lightRadiusref.m_Entity->ToString()));
		ComponentField& field = lightRadiusref.GetComponentFieldSafeMutable();
		//Assert(false, std::format("Light source ref field: {}", field.ToString()));

		playerEntity.AddComponent<AnimatorData>(AnimatorData(std::vector<AnimationPropertyVariant>{
			AnimationProperty<std::uint8_t>(lightRadiusref, {
			AnimationPropertyKeyframe<std::uint8_t>(std::uint8_t(8), 0),
			AnimationPropertyKeyframe<std::uint8_t>(std::uint8_t(1), 1) })}, 1, 1, true));

		SpriteAnimatorData& spriteAnimator= playerEntity.AddComponent<SpriteAnimatorData>(SpriteAnimatorData());
		SpriteAnimationAsset* testAnim = assetManager.TryGetTypeAssetFromPathMutable<SpriteAnimationAsset>("sprite_animations/test.sanim");
		//Assert(false, std::format("Adding anim:{}", testAnim->ToString()));
		spriteAnimator.AddAnimation(*testAnim);
		spriteAnimator.TryPlayAnimation(testAnim->GetAnimation().m_Name);

		//Note: the camera has to be moved back a little so objects at origin are still seen due to min near plane value= 0.1
		EntityData& mainCameraEntity = globalsManager.CreateGlobalEntity("MainCamera", TransformComponent(Vec3(0, 0, -0.1), Vec3::One(), Quat::Identity()));
		CameraComponent& cameraData = mainCameraEntity.AddComponent<CameraComponent>(CameraComponent{ CameraSettings{SCREEN_ASPECT_RATIO, 10, nullptr} });
		cameraController.TryRegisterCamera(cameraData);

		EntityData& trigger = globalsManager.CreateGlobalEntity("Trigger", TransformComponent(Vec3{15, 0, 0}));
		CollisionBoxData& triggerCollider = trigger.AddComponent<CollisionBoxData>(CollisionBoxData(Vec2(5, 5), Vec2(0, 0)));
		TriggerData& triggerData= trigger.AddComponent<TriggerData>(TriggerData(&triggerCollider));
	}
}

