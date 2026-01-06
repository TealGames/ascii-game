#include "Game/SceneCreator.hpp"
#include "Core/Scene/Scene.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Rendering/Material.hpp"
#include "Utils/MathAdvanced.hpp"
#include "ECS/Component/Types/World/Mesh3DComponent.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "Core/Asset/Model3dAsset.hpp"
#include "Core/Rendering/GraphicsManager.hpp"

namespace SceneCreator
{
	//Occurs when a scene is first loaded (only once on init)
	void OnSceneLoad(Scene& scene, AssetManagement::AssetManager& assetManager)
	{
		EntityData& monkey = scene.CreateEntity("Monkey", TransformComponent(Vec3(0, 0.1, 0), Vec3(0.1, 0.1, 0.1), Quat::Identity()));
		Model3dAsset* model = assetManager.TryGetTypeAssetFromPathMutable<Model3dAsset>("models/monkey" BASIC_MESH_EXTENSION);
		model->GetModelMutable().m_Objects[0].m_Material.SetSurface(1, 0, nullptr);
		monkey.AddComponent<Mesh3DComponent>(Mesh3DComponent(model->GetModelMutable(), 0));
	}

	//Occurs whenever a new scene becomes active
	void OnSceneStart(Scene& scene, AssetManagement::AssetManager& assetManager)
	{

	}
}