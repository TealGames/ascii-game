#include "pch.hpp"
#include "Entity.hpp"
#include "CameraSystem.hpp"
#include "Point2DInt.hpp"
#include "Updateable.hpp"
#include "Component.hpp"
#include "Globals.hpp"
#include "Globals.hpp"
#include "SceneManager.hpp"
//#include "GameRenderer.hpp"
#include "CartesianPosition.hpp"
#include "Array2DPosition.hpp"
#include "PositionConversions.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"
#include "ProfilerTimer.hpp"

namespace ECS
{
    static constexpr bool CACHE_LAST_BUFFER = true;
    static constexpr bool DO_SIZE_SCALING = true;
    static constexpr float FONT_SIZE_FACTOR = 0.1;

	CameraSystem::CameraSystem(ColliderOutlineBuffer* colliderBuffer, LineBuffer* lineBuffer) :
        m_currentFrameBuffer(), m_colliderOutlineBuffer(colliderBuffer), m_lineBuffer(lineBuffer)
	{
        Utils::LogWarning(std::format("CREATED CAMERA SYSTEM: {}", std::to_string(colliderBuffer!=nullptr)));
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
            //Utils::Log("NO camera render update");
            m_currentFrameBuffer = data.m_LastFrameBuffer;
            //Rendering::RenderBuffer(data.m_LastFrameBuffer.value());
            return;
        }

        scene.IncreaseFrameDirtyComponentCount();
        if (!data.m_CameraSettings.m_HasFixedPosition) UpdateCameraPosition(data, mainCamera);
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
        /*Utils::Log(std::format("Main camera trans: {} follow trans: {}", mainCamera.m_Transform.m_Pos.ToString(), 
            std::to_string(cameraData.m_CameraSettings.m_FollowTarget!=nullptr)));*/
        //Utils::Log(std::format("Main camera trans: {} follow trans: ", mainCamera.m_Transform.m_Pos.ToString()));
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

        Utils::LogWarning(std::format("COLLAPSING CAMERA with scale: {} wdith factor: {} height factoer: {}",
            std::to_string(scaleFactor), std::to_string(SCREEN_WIDTH / cameraData.m_CameraSettings.m_WorldViewportSize.m_X), 
            std::to_string(SCREEN_HEIGHT / cameraData.m_CameraSettings.m_WorldViewportSize.m_Y)));


        /*TextBuffer newBuffer = TextBuffer(cameraData.m_CameraSettings.m_ViewportSize.m_X, cameraData.m_CameraSettings.m_ViewportSize.m_Y, TextChar{});*/
        const std::vector<const RenderLayer*> layers = scene.GetAllLayers();
        //Utils::Log(std::format("Total layers: {}", std::to_string(layers.size())));
        ScreenPosition newScreenPos = {};
        for (const auto& layer : layers)
        {
            //Utils::Log(std::format("LAYER has buffer: {}", layer->ToString()));
            for (const auto& textBufferPos : layer->GetBuffer())
            {
                /*Utils::Log(std::format("Is pos {} within viewport: {}", textBufferPos.ToString(), 
                    std::to_string(IsWithinViewport(cameraData, textBufferPos.m_Pos))));*/
                if (!IsWithinViewport(cameraData, textBufferPos.m_Pos)) continue;
               /* Utils::Log(std::format("Convert pos: {} to screen pos; {}", 
                    textBufferPos.m_Pos.ToString(), WorldToScreenPosition(cameraData, textBufferPos.m_Pos).ToString()));*/

                m_currentFrameBuffer.emplace_back(textBufferPos);
                //TODO: it seems as thoguh font scaling causes problems and size inconsistencies with rest of world

                if (DO_SIZE_SCALING) m_currentFrameBuffer.back().m_FontData.m_FontSize *= scaleFactor;
                newScreenPos = Conversions::WorldToScreenPosition(cameraData, textBufferPos.m_Pos);
                m_currentFrameBuffer.back().m_Pos = Utils::Point2D(static_cast<float>(newScreenPos.m_X), static_cast<float>(newScreenPos.m_Y));
            }
        }

        if (DO_SIZE_SCALING)
        {
            if (m_colliderOutlineBuffer != nullptr && m_colliderOutlineBuffer->HasData())
            {
                //Utils::LogWarning("DOING SIZE SCALING");
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

        /*
        //Utils::Log(std::format("ONE LAYER IN CAMERA: {}", layers[0]->ToString()));
        //Utils::Log(std::format("SECOND LAYER IN CAMERA: {}", layers[1]->ToString()));

        const CartesianGridPosition cartesianTopLeft = Conversions::CartesianToGrid(mainCamera.m_Transform.m_Pos) - (cameraData.m_CameraSettings.m_WorldViewportSize / 2);
        const Array2DPosition renderCoordsTopLeft = Conversions::GridToArray(cartesianTopLeft);
        //Utils::Log(std::format("CAMERA: top left: {}", cartesianTopLeft.ToString()));

        //TODO: it seems like this process should proably be much quicker and maybe we should optimize
        //by first checking if the area is smaller and convering those automatically with empty chars
        Array2DPosition localPos = {};
        Array2DPosition globalPos = {};
        bool hasFoundChar = false;

        for (int r = 0; r < newBuffer.GetHeight(); r++)
        {
            for (int c = 0; c < newBuffer.GetWidth(); c++)
            {
                hasFoundChar = false;
                localPos = { r, c };
                globalPos = renderCoordsTopLeft + localPos;
                for (int layerIndex = layers.size() - 1; layerIndex >= 0; layerIndex--)
                {
                    const RenderLayer& layer = *layers[layerIndex];

                    //std::cout << std::format("Layer index: {}/{} of r: {} c:{} GLOBAL POS: {} valid: {}",
                    //std::to_string(layerIndex), std::to_string(layers.size()), std::to_string(r), 
                    //std::to_string(c), globalPos.ToString(), layer.m_SquaredTextBuffer.IsValidPos(globalPos)) << std::endl;

                    //Utils::Log(std::format("Attempting char at: {} of HEGIHT: {} WIDTH: {}",
                    //localPos.ToString(), std::to_string(layer.m_TextBuffer.m_HEIGHT), std::to_string(layer.m_TextBuffer.m_WIDTH)));
                    
                    if (!layer.m_SquaredTextBuffer.IsValidPos(globalPos)) continue;
                   
                    
                    const TextChar posChar = layer.m_SquaredTextBuffer.GetAtUnsafe(globalPos);
                    //if (posChar == nullptr) continue;
                    if (posChar.m_Char== EMPTY_CHAR_PLACEHOLDER) continue;

                    hasFoundChar = true;
                    newBuffer.SetAt(localPos, posChar);
                    //Utils::Log(std::format("Setting the position: {} of buffer with char: {}",
                     // localPos.ToString(), posChar.ToString()));
                   //Utils::Log(std::format("Found valid topmost pos at: {} with char: {} color: {} at layer: {} BUT WHEN DIRECT COLOR: {}", 
                   // globalPos.ToString(), Utils::ToString(posChar->m_Char), RaylibUtils::ToString(posChar->m_Color), 
                    //std::to_string(layerIndex), layer.m_SquaredTextBuffer.ToString(false)));
                    break;
                }

                //Since we may have the camera go out of bounds at times to fit player at desired pos, or
                //we may have smaller scene than camera size, we add invalid pos as empty chars
                if (!hasFoundChar)
                {
                    newBuffer.SetAt(localPos, TextChar{ Color(), EMPTY_CHAR_PLACEHOLDER });
                    //Utils::Log("OUT OF BOUNDS SET EMPTYx");
                }
            }
        }
        */
        //Utils::Log(std::format("Camera collapsing has buffer: {}", newBuffer.ToString(false)));
        //return newBuffer;
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
