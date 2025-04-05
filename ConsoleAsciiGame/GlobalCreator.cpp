#include "pch.hpp"
#include "GlobalCreator.hpp"
#include "GlobalEntityManager.hpp"
#include "Globals.hpp"
#include "LightSourceData.hpp"
#include "EntityRendererData.hpp"
#include "AnimatorData.hpp"
#include "CameraData.hpp"
#include "CollisionBoxData.hpp"
#include "SpriteAnimatorData.hpp"
#include "PhysicsBodyData.hpp"
#include "PlayerData.hpp"
#include "SceneManager.hpp"
#include "CameraController.hpp"
#include "ParticleEmitterData.hpp"
#include "RaylibUtils.hpp"

namespace GlobalCreator
{
	void CreateGlobals(GlobalEntityManager& globalsManager, SceneManagement::SceneManager& sceneManager, CameraController& cameraController)
	{
		//ECS::Entity& obstacle = m_sceneManager.GetActiveSceneMutable()->CreateEntity("obstacle", TransformData(Vec2{ 20, 20 }));

		//Font* fontptr = &(GetGlobalFont());
		VisualDataPreset visualPreset = {GetGlobalFont(), VisualData::DEFAULT_FONT_SIZE, VisualData::DEFAULT_CHAR_SPACING,
				CharAreaType::Predefined, VisualData::DEFAULT_PREDEFINED_CHAR_AREA, VisualData::DEFAULT_PIVOT };
		//LogError(std::format("Is valid preset font:{}", std::to_string(RaylibUtils::IsValidFont(visualPreset.m_Font))));


		ECS::Entity& playerEntity = globalsManager.CreateGlobalEntity("player", TransformData(Vec2{ 10, 5 }));
		CollisionBoxData& playerCollider = playerEntity.AddComponent<CollisionBoxData>(CollisionBoxData(playerEntity.m_Transform, Vec2(2, 2), Vec2(0, 0)));
		PhysicsBodyData& playerRB = playerEntity.AddComponent<PhysicsBodyData>(PhysicsBodyData(playerCollider, 1, GRAVITY, 20));

		PlayerData& playerData = playerEntity.AddComponent<PlayerData>(PlayerData(playerRB, 5, 20));

		//InputData& inputData = playerEntity.AddComponent<InputData>(InputData{});
		LightSourceData& lightSource = playerEntity.AddComponent<LightSourceData>(LightSourceData{ 8, RenderLayerType::Background,
			ColorGradient(Color(243, 208, 67, 255), Color(228, 8, 10, 255)), std::uint8_t(254), 1.2f });

		//Assert(false, std::format("player light source:{}", playerEntity.TryGetComponentWithName("LightSourceData")->ToString()));

		//Log("CREATING PLAYER RB");
		//ComponentData* ptr = &lightSource;
		//Assert(false, std::format("LIGHT SOURCE PTR:{}", typeid(*ptr).name()));
		//Assert(false, std::format("Light source: {}", typeid(*ptr).name()));
		//Assert(false, std::format("Light source fields: {}", lightSource.ToStringFields()));

		playerEntity.AddComponent<EntityRendererData>(EntityRendererData{
			VisualData({ {TextCharPosition({0,0}, TextChar(GRAY, 'H')) } },visualPreset), RenderLayerType::Player });

		ComponentFieldReference lightRadiusref = ComponentFieldReference(&lightSource, "Radius");
		//Assert(false, std::format("Entity light radius: {}", lightRadiusref.m_Entity->ToString()));
		ComponentField& field = lightRadiusref.GetComponentFieldSafeMutable();
		//Assert(false, std::format("Light source ref field: {}", field.ToString()));

		playerEntity.AddComponent<AnimatorData>(AnimatorData(std::vector<AnimationPropertyVariant>{
			AnimationProperty<std::uint8_t>(lightRadiusref, {
			AnimationPropertyKeyframe<std::uint8_t>(std::uint8_t(8), 0),
			AnimationPropertyKeyframe<std::uint8_t>(std::uint8_t(1), 1) })}, 1, 1, true));

		playerEntity.AddComponent<SpriteAnimatorData>(SpriteAnimatorData(
			{ SpriteAnimationFrame(0, VisualData(RawTextBufferBlock{{TextCharPosition({}, TextChar(WHITE, 'O'))}}, visualPreset)),
			  SpriteAnimationFrame(2, VisualData(RawTextBufferBlock{{TextCharPosition({}, TextChar(WHITE, '4'))}}, visualPreset)) }, 1, 4, true));

		ColorGradient particleGradient = ColorGradient(RED, BLUE);
		playerEntity.AddComponent<ParticleEmitterData>(ParticleEmitterData('W', FloatRange(1, 11), particleGradient, FontData(5, GetGlobalFont()),
			RenderLayerType::Player, WorldPosition(), FloatRange(5, 16), float(5)));

		//m_playerInfo = ECS::EntityComponents<PlayerData, PhysicsBodyData>{ playerEntity, playerData, playerRB };

		ECS::Entity& mainCameraEntity = globalsManager.CreateGlobalEntity("MainCamera", TransformData(Vec2{ 0, 0 }));
		CameraData& cameraData = mainCameraEntity.AddComponent<CameraData>(CameraData{ CameraSettings{SCREEN_ASPECT_RATIO, 120, &playerEntity} });
		cameraController.TryRegisterCamera(cameraData);

		////This is to make sure they all have the main camera set as this global
		//sceneManager.m_OnLoad.AddListener([&mainCameraEntity](Scene* scene)-> void
		//	{
		//		if (scene == nullptr) return;
		//		scene->SetMainCamera(mainCameraEntity);
		//	});
		////m_mainCameraInfo = ECS::EntityComponentPair<CameraData>{ mainCameraEntity, cameraData };
	}
}

