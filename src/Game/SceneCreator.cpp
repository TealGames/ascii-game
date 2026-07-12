#include "Game/SceneCreator.hpp"
#include "StaticGlobals.hpp"
#include "Core/Scene/Scene.hpp"
#include "Core/EngineState.hpp"
#include "Core/Asset/SceneAsset.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "Core/Rendering/GraphicsManager.hpp"
#include "Core/Rendering/Material.hpp"
#include "Math/Math3d.hpp"
#include "ECS/Component/Types/World/Mesh3DComponent.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "Core/Asset/Model3dAsset.hpp"
#include "ECS/Component/Types/World/PointLight3DComponent.hpp"

namespace Engine::Scenes::SceneCreator
{
	static Core::EngineState* EngineState = nullptr;

	void Init(Core::EngineState& state)
	{
		EngineState = &state;
		EngineState->m_SceneManager->m_OnSceneAssetLoad.AddListener(OnSceneAssetLoad);
		EngineState->m_SceneManager->m_OnActiveSceneChange.AddListener(OnActiveSceneChange);
	}

	//Occurs when a scene is first loaded (only once on init)
	void OnSceneAssetLoad(Scene* scene)
	{	
		/*
		EntityData& town = scene.CreateEntity("Town", TransformComponent(Vec3(0, 0, 0), Vec3(1, 1, 1), Quat::Identity()));
		Model3dAsset* townModel = state.m_AssetManager->TryGetTypeAssetFromPathMutable<Model3dAsset>("models/japantown" BASIC_MESH_EXTENSION);
		if (townModel != nullptr)
		{
			LogError(std::format("Town model: {}", townModel->GetModel().ToString()));
		}
		//model->GetModelMutable().m_Objects[0].m_Material.SetSurface(1, 0, nullptr);
		town.AddComponent<Model3>(Mesh3DComponent(townModel->GetModelMutable(), 0));
		town.m_IsImmovable = true;

		EntityData& globalLight = scene.CreateEntity("Sun", TransformComponent(Vec3(0, 10, 0), Vec3(1,1,1), Quat::Identity()));
		globalLight.AddComponent<PointLight3DComponent>();
		globalLight.m_IsImmovable = true;
		*/

		ECS::EntityData& monkey = scene->CreateEntity("Monkey", TransformComponent(Vec3(0, 0.1, 0), Vec3(0.2, 0.2, 0.2), Math::Quat::Identity()));
		Rendering::Model3dAsset* modelAsset = EngineState->m_AssetManager->TryGetTypeAssetFromPathMutable<Rendering::Model3dAsset>(
			"models/monkey" BASIC_MESH_EXTENSION);
		modelAsset->GetModelMutable().m_Objects[0].m_MaterialAsset->GetMaterialMutable().SetSurface(1, 0, nullptr);
		monkey.AddComponent(Rendering::Mesh3DComponent(*modelAsset, 0));
		monkey.m_IsImmovable = true;

		ECS::EntityData& floor = scene->CreateEntity("CheckerboardFloor", TransformComponent(Vec3(0, 0, 0), Vec3::One(), Math::Quat::Identity()));
		Rendering::MaterialAsset* checkerboardMaterialAsset = EngineState->m_GraphicsContext.m_GraphicsManager->TryGetMaterialAssetMutable("checkerboard");
		Rendering::Model3dAsset* planeAsset = EngineState->m_GraphicsContext.m_GraphicsManager->TryGetBasicMeshAssetMutable(Rendering::BasicMeshType::Plane);

		floor.AddComponent(Rendering::Mesh3DComponent(*planeAsset, 0, checkerboardMaterialAsset));
		floor.m_IsImmovable = true;

		constexpr float planeSize = 1;

		ECS::EntityData& wallLeft = scene->CreateEntity("WallLeft", TransformComponent(Vec3(-planeSize / 2, planeSize / 2, 0),
													Vec3::One(), Math::ToQuaternion(Vec3(0, 0, ::Math::RAD_90))));
		Rendering::MaterialAsset* redMaterialAsset = EngineState->m_GraphicsContext.m_GraphicsManager->TryGetMaterialAssetMutable("red");
		Rendering::Mesh3DComponent& wallleftMesh = wallLeft.AddComponent(Rendering::Mesh3DComponent(*planeAsset, 0, redMaterialAsset));
		wallLeft.m_IsImmovable = true;

		ECS::EntityData& wallRight = scene->CreateEntity("WallRight", TransformComponent(Vec3(planeSize/2, planeSize/2, 0),
													Vec3::One(), Math::ToQuaternion(Vec3(0, 0, ::Math::RAD_270))));
		Rendering::MaterialAsset* greenMaterialAsset = EngineState->m_GraphicsContext.m_GraphicsManager->TryGetMaterialAssetMutable("green");
		wallRight.AddComponent(Rendering::Mesh3DComponent(*planeAsset, 0, greenMaterialAsset));
		wallRight.m_IsImmovable = true;

		
		ECS::EntityData& wallBack = scene->CreateEntity("WallBack", TransformComponent(Vec3(0, planeSize/2, -planeSize/2),
													Vec3::One(), Math::ToQuaternion(Vec3(::Math::RAD_270, 0, 0))));
		Rendering::MaterialAsset* whiteMaterialAsset = EngineState->m_GraphicsContext.m_GraphicsManager->TryGetMaterialAssetMutable("white");
		wallBack.AddComponent(Rendering::Mesh3DComponent(*planeAsset, 0, whiteMaterialAsset));
		wallBack.m_IsImmovable = true;

		ECS::EntityData& roof = scene->CreateEntity("Roof", TransformComponent(Vec3(0, planeSize/2, 0), Vec3::One(),
														Math::ToQuaternion(Vec3(::Math::RAD_180, 0, 0))));
		Rendering::MaterialAsset* whiteLitMaterialAsset = EngineState->m_GraphicsContext.m_GraphicsManager->TryGetMaterialAssetMutable("white_lit");
		roof.AddComponent(Rendering::Mesh3DComponent(*planeAsset, 0, whiteLitMaterialAsset));
		roof.m_IsImmovable = true;

		const std::filesystem::path path = SCENE_ASSET_DIR "scene1.json";
		SaveSceneToPath(*scene, path);
		LogError("SAVED");
	}

	//Occurs whenever a new scene becomes active
	void OnActiveSceneChange(Scene* scene)
	{
		/*if (!state.m_GraphicsContext.m_GraphicsManager->TrySetSkybox("textures/skybox_stylized_night.png"))
		{
			LogError(std::format("Failed to set scene:{} skybox", scene.GetName()));
		}*/
	}
}