#include "pch.hpp"
#include "ECS/Systems/Types/World/CameraSystem.hpp"
#include "ECS/Component/Component.hpp"
#include "StaticGlobals.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "Utils/Data/Array2DPosition.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Utils/RaylibUtils.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "Core/Rendering/GameRenderer.hpp"
#include "Core/Asset/FontAsset.hpp"
#include "Utils/RaylibUtils.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 

namespace ECS
{
    static constexpr bool CACHE_LAST_BUFFER = true;
    static constexpr bool DO_SIZE_SCALING = true;
    static constexpr float FONT_SIZE_FACTOR = 0.1;

	CameraSystem::CameraSystem(Rendering::Renderer& renderer) :
        m_renderer(&renderer)//, m_currentFrameBuffer(), m_colliderOutlineBuffer(colliderBuffer), m_lineBuffer(lineBuffer)
	{
        GlobalComponentInfo::AddComponentInfo(typeid(CameraData), ComponentInfo(DependencyType::Entity));
	}

    void CameraSystem::SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime)
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("CameraSystem::SystemUpdate");
#endif 

        scene.IncreaseFrameDirtyComponentCount();
        if (!mainCamera.m_CameraSettings.HasNoFollowTarget()) UpdateCameraPosition(mainCamera);

        CollapseLayersWithinViewport(scene, mainCamera);
    }

    //TODO: this should be modified to have a follow delay, lookeahead blocks, etc to be more dynamic
    void CameraSystem::UpdateCameraPosition(CameraData& cameraData)
    {
        cameraData.GetEntityMutable().GetTransformMutable().m_LocalPos= cameraData.m_CameraSettings.m_FollowTarget->GetTransform().GetGlobalPos();
    }

    void CameraSystem::CollapseLayersWithinViewport(const Scene& scene, CameraData& cameraData)
    {
        const TransformData& cameraTransform = cameraData.GetEntity().GetTransform();
        float scaleFactor = std::max(SCREEN_WIDTH/cameraData.m_CameraSettings.m_WorldViewportSize.m_X, 
                                     SCREEN_HEIGHT / cameraData.m_CameraSettings.m_WorldViewportSize.m_Y);

        const std::vector<const RenderLayer*> layers = scene.GetAllLayers();
        ScreenPosition newScreenPos = {};
        Vec2 screenSize = {};

        std::array<InfinitePlane3D, 6> viewPlanes = cameraData.CalculateFrustumPlanes();

        //TODO: this is inefficient because we render each pos within viewport, but even if some objects are within the same pos
        //the one behind it is still rendered. NOTE: it is difficult to find a solution when we might have small overlaps and we 
        //wanbt overlaps to be visible to ensure realism/not akward visuals + makes it difficult when using raylib
        for (const auto& layer : layers)
        {
            //Log(std::format("LAYER has buffer: {}", layer->ToString()));
            //LogWarning(std::format("Found layer with buffer size:{}", layer->GetBuffer().size()));
            for (const auto& textBufferPos : layer->GetBuffer())
            {
                if (!cameraData.DoesViewVolumeContainPosOptimized(textBufferPos.m_Pos, &viewPlanes))
                    continue;
             
                //TODO: add rendering
                /*
                newScreenPos = cameraData.WorldToScreenPosition(textBufferPos.m_Pos);
                screenSize = cameraData.WorldToScreenSize(textBufferPos.m_FontData.m_RectSize);

                m_renderer->AddTextCall(newScreenPos, textBufferPos.m_FontData.m_FontAsset->GetFont(), textBufferPos.m_Text.m_Char,
                   GetBestFontSize(textBufferPos.m_FontData.m_FontAsset->GetFont(), textBufferPos.m_FontData.m_Tracking, screenSize, textBufferPos.m_Text.m_Char), 
                    textBufferPos.m_FontData.m_Tracking, textBufferPos.m_Text.m_Color);
                    */
            }
        }
    }
}
