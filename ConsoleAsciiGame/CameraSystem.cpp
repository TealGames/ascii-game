#include "pch.hpp"
#include "Entity.hpp"
#include "CameraSystem.hpp"
#include "Updateable.hpp"
#include "Component.hpp"
#include "Globals.hpp"
#include "SceneManager.hpp"
#include "Array2DPosition.hpp"
#include "PositionConversions.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"

#ifdef ENABLE_PROFILER
#include "ProfilerTimer.hpp"
#endif 

namespace ECS
{
    static constexpr bool CACHE_LAST_BUFFER = true;
    static constexpr bool DO_SIZE_SCALING = true;
    static constexpr float FONT_SIZE_FACTOR = 0.1;

	CameraSystem::CameraSystem(ColliderOutlineBuffer* colliderBuffer, LineBuffer* lineBuffer) :
        m_currentFrameBuffer(), m_colliderOutlineBuffer(colliderBuffer), m_lineBuffer(lineBuffer)
	{
        LogWarning(std::format("CREATED CAMERA SYSTEM: {}", std::to_string(colliderBuffer!=nullptr)));
	}

    void CameraSystem::SystemUpdate(Scene& scene, CameraData& data,
        ECS::Entity& mainCamera, const float& deltaTime)
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("CameraSystem::SystemUpdate");
#endif 

        m_currentFrameBuffer.clear();

        //TODO: somthing was wrong with dirty counting )(most likely) so the optimization was not working
        //so instead use dirty from entities not from componentns
        if (CACHE_LAST_BUFFER && !scene.HasDirtyComponents() && !data.m_LastFrameBuffer.empty())
        {
            //Log("NO camera render update");
            m_currentFrameBuffer = data.m_LastFrameBuffer;
            //Rendering::RenderBuffer(data.m_LastFrameBuffer.value());
            return;
        }

        scene.IncreaseFrameDirtyComponentCount();
        if (!data.m_CameraSettings.HasFixedPosition()) UpdateCameraPosition(data, mainCamera);
       /* Rendering::RenderBuffer(TextBuffer(5, 5, TextChar(BLUE, 'V')));
        data.m_LastFrameBuffer = TextBuffer(5, 5, TextChar(BLUE, 'V'));
        data.m_LastFrameBuffer = CollapseLayersWithinViewport(scene, data, mainCamera);*/
        //return;

        CollapseLayersWithinViewport(scene, data, mainCamera);
        //m_currentFrameBuffer = CollapseLayersWithinViewport(scene, data, mainCamera);
        if (CACHE_LAST_BUFFER) data.m_LastFrameBuffer = m_currentFrameBuffer;
    }

    //TODO: this should be modified to have a follow delay, lookeahead blocks, etc to be more dynamic
    void CameraSystem::UpdateCameraPosition(CameraData& cameraData, ECS::Entity& mainCamera)
    {
        /*Log(std::format("Main camera trans: {} follow trans: {}", mainCamera.m_Transform.m_Pos.ToString(), 
            std::to_string(cameraData.m_CameraSettings.m_FollowTarget!=nullptr)));*/
        //Log(std::format("Main camera trans: {} follow trans: ", mainCamera.m_Transform.m_Pos.ToString()));
        mainCamera.m_Transform.SetPos(cameraData.m_CameraSettings.m_FollowTarget->m_Transform.m_Pos);
    }

    bool CameraSystem::IsWithinViewport(const CameraData& camera, const WorldPosition& pos) const
    {
        WorldPosition bottomLeftPos = camera.m_Entity->m_Transform.m_Pos - (camera.m_CameraSettings.m_WorldViewportSize/2);
        WorldPosition topRightPos = camera.m_Entity->m_Transform.m_Pos + (camera.m_CameraSettings.m_WorldViewportSize/2);
        return bottomLeftPos.m_X <= pos.m_X && pos.m_X <= topRightPos.m_X && bottomLeftPos.m_Y <= pos.m_Y && pos.m_Y <= topRightPos.m_Y;
    }

    void CameraSystem::CollapseLayersWithinViewport(const Scene& scene, CameraData& cameraData, ECS::Entity& mainCamera)
    {
        //TODO: this is cuainsg some performance rpboelms
        m_currentFrameBuffer.clear();

        float scaleFactor = std::max(SCREEN_WIDTH/cameraData.m_CameraSettings.m_WorldViewportSize.m_X, 
                                     SCREEN_HEIGHT / cameraData.m_CameraSettings.m_WorldViewportSize.m_Y);

        LogWarning(std::format("COLLAPSING CAMERA with scale: {} wdith factor: {} height factoer: {}",
            std::to_string(scaleFactor), std::to_string(SCREEN_WIDTH / cameraData.m_CameraSettings.m_WorldViewportSize.m_X), 
            std::to_string(SCREEN_HEIGHT / cameraData.m_CameraSettings.m_WorldViewportSize.m_Y)));

        const std::vector<const RenderLayer*> layers = scene.GetAllLayers();
        //Log(std::format("Total layers: {}", std::to_string(layers.size())));
        ScreenPosition newScreenPos = {};
        for (const auto& layer : layers)
        {
            //Log(std::format("LAYER has buffer: {}", layer->ToString()));
            for (const auto& textBufferPos : layer->GetBuffer())
            {
                /*Log(std::format("Is pos {} within viewport: {}", textBufferPos.ToString(), 
                    std::to_string(IsWithinViewport(cameraData, textBufferPos.m_Pos))));*/
                if (!IsWithinViewport(cameraData, textBufferPos.m_Pos)) continue;
               /* Log(std::format("Convert pos: {} to screen pos; {}", 
                    textBufferPos.m_Pos.ToString(), WorldToScreenPosition(cameraData, textBufferPos.m_Pos).ToString()));*/

                m_currentFrameBuffer.emplace_back(textBufferPos);
                //TODO: it seems as thoguh font scaling causes problems and size inconsistencies with rest of world

                if (DO_SIZE_SCALING) m_currentFrameBuffer.back().m_FontData.m_FontSize *= scaleFactor;
                newScreenPos = Conversions::WorldToScreenPosition(cameraData, textBufferPos.m_Pos);
                m_currentFrameBuffer.back().m_Pos = Vec2(static_cast<float>(newScreenPos.m_X), static_cast<float>(newScreenPos.m_Y));
            }
        }

        if (DO_SIZE_SCALING)
        {
            if (m_colliderOutlineBuffer != nullptr && m_colliderOutlineBuffer->HasData())
            {
                //LogWarning("DOING SIZE SCALING");
                for (auto& outline : m_colliderOutlineBuffer->m_RectangleBuffer)
                {
                    outline.m_Size = outline.m_Size * scaleFactor;
                }
            }
        }

        if (m_lineBuffer != nullptr && !m_lineBuffer->empty())
        {
            for (auto& line : *m_lineBuffer)
            {
                ScreenPosition startScreenPos = Conversions::WorldToScreenPosition(cameraData, line.m_StartPos);
                line.m_StartPos = { static_cast<float>(startScreenPos.m_X), static_cast<float>(startScreenPos.m_Y)};

                ScreenPosition endScreenPos = Conversions::WorldToScreenPosition(cameraData, line.m_EndPos);
                line.m_EndPos = { static_cast<float>(endScreenPos.m_X), static_cast<float>(endScreenPos.m_Y) };
            }
        }

        if (CACHE_LAST_BUFFER) cameraData.m_LastFrameBuffer = m_currentFrameBuffer;
    }
    
    const TextBufferMixed& CameraSystem::GetCurrentFrameBuffer() const
    {
        if (m_currentFrameBuffer.empty()) return {};
        return m_currentFrameBuffer;
    }

    const ColliderOutlineBuffer* CameraSystem::GetCurrentColliderOutlineBuffer() const
    {
        return m_colliderOutlineBuffer;
    }

    const LineBuffer* CameraSystem::GetCurrentLineBuffer() const
    {
        return m_lineBuffer;
    }
}
