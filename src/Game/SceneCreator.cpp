#include "Game/SceneCreator.hpp"
#include "Core/Scene/Scene.hpp"
#include "Core/EngineState.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Rendering/GraphicsManager.hpp"
#include "Core/Rendering/Material.hpp"
#include "Utils/MathAdvanced.hpp"
#include "ECS/Component/Types/World/Mesh3DComponent.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "Core/Asset/Model3dAsset.hpp"
#include "Core/Rendering/GraphicsManager.hpp"

namespace SceneCreator
{
	//Occurs when a scene is first loaded (only once on init)
	void OnSceneLoad(Scene& scene, EngineState& state)
	{
		EntityData& monkey = scene.CreateEntity("Monkey", TransformComponent(Vec3(0, 0.1, 0), Vec3(0.2, 0.2, 0.2), Quat::Identity()));
		Model3dAsset* model = state.m_AssetManager->TryGetTypeAssetFromPathMutable<Model3dAsset>("models/monkey" BASIC_MESH_EXTENSION);
		model->GetModelMutable().m_Objects[0].m_Material.SetSurface(1, 0, nullptr);
		monkey.AddComponent<Mesh3DComponent>(Mesh3DComponent(model->GetModelMutable(), 0));
		monkey.m_IsImmovable = true;

		EntityData& floor = scene.CreateEntity("CheckerboardFloor", TransformComponent(Vec3(0, 0, 0), Vec3::One(), Quat::Identity()));
		Rendering::Texture& checkerboardTexture = state.m_AssetManager->TryGetTypeAssetFromPathMutable<TextureAsset>
																		("textures/checkerboard.jpg")->GetTextureMutable();
		Rendering::Material* floorMaterial = state.m_GraphicsContext.m_GraphicsManager->TryCreateRuntimeMaterial(Rendering::Material("Checkerboard",
			&checkerboardTexture, HDRColor(0.2f, 0.2f, 0.2f, 1.0f), 1, HDRColor(0.0f, 0.0f, 0.0f, 0.0f), 0.1, 0));
		Rendering::Model3d* planeModel = state.m_GraphicsContext.m_GraphicsManager->TryGetBasicMeshMutable(Rendering::BasicMeshType::Plane);

		floor.AddComponent<Mesh3DComponent>(Mesh3DComponent(*planeModel, 0, floorMaterial));
		floor.m_IsImmovable = true;

		constexpr float planeSize = 1;

		EntityData& wallLeft = scene.CreateEntity("WallLeft", TransformComponent(Vec3(-planeSize / 2, planeSize / 2, 0),
													Vec3::One(), ToQuaternion(Vec3(0, 0, RAD_90))));
		Rendering::Material* wallLeftMaterial = state.m_GraphicsContext.m_GraphicsManager->TryCreateRuntimeMaterial(
			Rendering::Material("WallLeft", nullptr, COLOR_RED, 1));
		Mesh3DComponent& wallleftMesh = wallLeft.AddComponent<Mesh3DComponent>(Mesh3DComponent(*planeModel, 0, wallLeftMaterial));
		wallLeft.m_IsImmovable = true;

		EntityData& wallRight = scene.CreateEntity("WallRight", TransformComponent(Vec3(planeSize/2, planeSize/2, 0),
													Vec3::One(), ToQuaternion(Vec3(0, 0, RAD_270))));
		Rendering::Material* wallRightMaterial = state.m_GraphicsContext.m_GraphicsManager->TryCreateRuntimeMaterial(
			Rendering::Material("WallRight", nullptr, COLOR_GREEN, 1));
		wallRight.AddComponent<Mesh3DComponent>(Mesh3DComponent(*planeModel, 0, wallRightMaterial));
		wallRight.m_IsImmovable = true;

		
		EntityData& wallBack = scene.CreateEntity("WallBack", TransformComponent(Vec3(0, planeSize/2, -planeSize/2),
													Vec3::One(), ToQuaternion(Vec3(RAD_270, 0, 0))));
		Rendering::Material* wallBackMaterial = state.m_GraphicsContext.m_GraphicsManager->TryCreateRuntimeMaterial(
			Rendering::Material("WallBack", nullptr, COLOR_WHITE, 1));
		wallBack.AddComponent<Mesh3DComponent>(Mesh3DComponent(*planeModel, 0, wallBackMaterial));
		wallBack.m_IsImmovable = true;

		EntityData& roof = scene.CreateEntity("Roof", TransformComponent(Vec3(0, planeSize/2, 0), Vec3::One(),
														ToQuaternion(Vec3(RAD_180, 0, 0))));
		Rendering::Material* roofMaterial = state.m_GraphicsContext.m_GraphicsManager->TryCreateRuntimeMaterial(
			Rendering::Material("Roof", nullptr, COLOR_WHITE, 1, HDRColor(5.0f, 5.0f, 5.0f, 1.0f)));
		roof.AddComponent<Mesh3DComponent>(Mesh3DComponent(*planeModel, 0, roofMaterial));
		roof.m_IsImmovable = true;
	}

	//Occurs whenever a new scene becomes active
	void OnSceneStart(Scene& scene, EngineState& state)
	{

	}
}