#include "pch.hpp"
#include "ECS/Systems/Types/World/CameraSystem.hpp"
#include "ECS/Component/Component.hpp"
#include "StaticGlobals.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "Core/Primitives/Array2DPosition.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "Core/Rendering/Renderer3d.hpp"
#include "Core/Asset/FontAsset.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 

namespace Engine::Camera
{
    static constexpr bool CACHE_LAST_BUFFER = true;
    static constexpr bool DO_SIZE_SCALING = true;
    static constexpr float FONT_SIZE_FACTOR = 0.1;

	CameraSystem::CameraSystem(Rendering::Renderer& renderer) :
        m_renderer(&renderer)//, m_currentFrameBuffer(), m_colliderOutlineBuffer(colliderBuffer), m_lineBuffer(lineBuffer)
	{
        ECS::GlobalComponentInfo::AddComponentInfo(typeid(CameraComponent), ECS::ComponentInfo(ECS::DependencyType::Entity, {}, 
            [this](EntityData& entity)-> void
            {
                CameraComponent* camera = entity.TryGetComponentMutable<CameraComponent>();
                entity.GetTransformMutable().m_DirtyCallback = [camera](const ECS::DirtyFlag) -> void 
                    {
                        camera->SetDirtyFlag(CameraComponent::VIEW_MATRIX_DIRTY_FLAG);
                    };
            }));
	}

    void CameraSystem::SystemUpdate(Scenes::Scene& scene, CameraComponent& mainCamera, const float& deltaTime)
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("CameraSystem::SystemUpdate");
#endif 

        if (mainCamera.HasFollowTarget()) UpdateCameraPosition(mainCamera);
        mainCamera.m_lastUpdateData.m_UpdatedThisFrame = CameraPrecalculatedDataUpdate::None;
        //CollapseLayersWithinViewport(scene, mainCamera);
    }

    //TODO: this should be modified to have a follow delay, lookeahead blocks, etc to be more dynamic
    void CameraSystem::UpdateCameraPosition(CameraComponent& cameraData)
    {
        //Note: we only set x y so that the depth is not messed up with objects moving farther or towards
        cameraData.GetEntityMutable().GetTransformMutable().GetLocalPosMutable().
            SetXY(cameraData.GetFollowTarget()->GetTransform().GetWorldPos().GetXY());
    }
}
