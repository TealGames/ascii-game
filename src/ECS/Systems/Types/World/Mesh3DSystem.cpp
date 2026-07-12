#include "ECS/Systems/Types/World/Mesh3DSystem.hpp"
#include "ECS/Component/Types/World/Mesh3DComponent.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/Types/World/CameraComponent.hpp"
#include "Core/Rendering/Renderer3d.hpp"
#include "Core/EngineState.hpp"
#include "Core/Rendering/GraphicsManager.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 

namespace Engine::Rendering
{
	Mesh3DSystem::Mesh3DSystem(Rendering::Renderer& renderer, const Core::EngineState& engineState) 
		: m_renderer(&renderer), m_engineState(&engineState) {}

	void Mesh3DSystem::SystemStart(Scenes::Scene& scene)
	{
		//All non movable entities will not change pos and thus do not need their vertices re-added every frame
		//TODO: this is problematic because static objects can still have other settings mutated, including visual changes like meshes, materials
		//which still would change the instance data (although might not change vertex data)
		scene.OperateOnComponents<Mesh3DComponent>(ECS::ComponentStateFlag::EntityActive | ECS::ANY_SERIALIZABLE_FLAG |
			ECS::ComponentStateFlag::EntityImmovable | ECS::ComponentStateFlag::ComponentEnabled,
			[this](Mesh3DComponent& component) -> void {ComponentUpdate(component); });
	}

	void Mesh3DSystem::ComponentUpdate(Mesh3DComponent& component)
	{
		const Rendering::ModelMesh* mesh = component.GetMesh();
		if (mesh == nullptr)
			return;

		if (component.m_MaterialAsset == nullptr)
		{
			component.m_MaterialAsset = m_engineState->m_GraphicsContext.m_GraphicsManager->GetDefaultMaterialAssetMutable();
		}

		//NOTE: since we do apply model transform to vertices on upload, we do NOT need to multiply the transform matrix
		//by the imported model's matrix
		m_renderer->AddCallMesh(*mesh, component.m_MaterialAsset->GetMaterialMutable(), component.GetTransform().GetWorldModelMatrix());
	}

	void Mesh3DSystem::SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("Mesh3dSystem::SystemUpdate");
#endif 

		scene.OperateOnComponents<Mesh3DComponent>(ECS::ComponentStateFlag::EntityActive | ECS::ANY_SERIALIZABLE_FLAG |
			ECS::ComponentStateFlag::EntityMovable | ECS::ComponentStateFlag::ComponentEnabled, 
			[this](Mesh3DComponent& component) -> void {ComponentUpdate(component); });
	}
}