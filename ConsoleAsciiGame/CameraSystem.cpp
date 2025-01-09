#include "pch.hpp"
#include "Entity.hpp"
#include "CameraSystem.hpp"
#include "Point2DInt.hpp"
#include "Updateable.hpp"
#include "Component.hpp"
#include "Globals.hpp"
#include "SceneManager.hpp"
//#include "GameRenderer.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"

namespace ECS
{
    static constexpr bool CACHE_LAST_BUFFER = true;

	CameraSystem::CameraSystem(TransformSystem& transformSystem) :
        m_transformSystem(transformSystem)
	{
	}

    void CameraSystem::SystemUpdate(Scene& scene, CameraData& data,
        ECS::Entity& mainCamera, const float& deltaTime)
    {
        m_currentFrameBuffer = std::nullopt;

        //TODO: somthing was wrong with dirty counting )(most likely) so the optimization was not working
        //so instead use dirty from entities not from componentns
        if (CACHE_LAST_BUFFER && !scene.HasDirtyComponents() && data.m_LastFrameBuffer.has_value())
        {
            Utils::Log("NO camera render update");
            m_currentFrameBuffer = data.m_LastFrameBuffer.value();
            //Rendering::RenderBuffer(data.m_LastFrameBuffer.value());
            return;
        }

        scene.IncreaseFrameDirtyComponentCount();
        if (!data.m_CameraSettings.m_HasFixedPosition) UpdateCameraPosition(data, mainCamera);
       /* Rendering::RenderBuffer(TextBuffer(5, 5, TextChar(BLUE, 'V')));
        data.m_LastFrameBuffer = TextBuffer(5, 5, TextChar(BLUE, 'V'));
        data.m_LastFrameBuffer = CollapseLayersWithinViewport(scene, data, mainCamera);*/
        //return;
      

        //Rendering::RenderBuffer(collapsedBuffer);
        m_currentFrameBuffer = CollapseLayersWithinViewport(scene, data, mainCamera);
        if (CACHE_LAST_BUFFER) data.m_LastFrameBuffer = m_currentFrameBuffer;
    }

    //TODO: this should be modified to have a follow delay, lookeahead blocks, etc to be more dynamic
    void CameraSystem::UpdateCameraPosition(CameraData& cameraData, ECS::Entity& mainCamera)
    {
        Utils::Log(std::format("Main camera trans: {} follow trans: {}", mainCamera.m_Transform.m_Pos.ToString(), 
            std::to_string(cameraData.m_CameraSettings.m_FollowTarget!=nullptr)));
        //Utils::Log(std::format("Main camera trans: {} follow trans: ", mainCamera.m_Transform.m_Pos.ToString()));
        m_transformSystem.SetPos(mainCamera.m_Transform,
            cameraData.m_CameraSettings.m_FollowTarget->m_Transform.m_Pos);
    }

    TextBuffer CameraSystem::CollapseLayersWithinViewport(const Scene& scene, CameraData& cameraData, ECS::Entity& mainCamera) const
    {
        TextBuffer newBuffer = TextBuffer(cameraData.m_CameraSettings.m_ViewportSize.m_X, cameraData.m_CameraSettings.m_ViewportSize.m_Y, TextChar{});
        const std::vector<const RenderLayer*> layers = scene.GetAllLayers();

        const Utils::Point2DInt cartesianTopLeft = mainCamera.m_Transform.m_Pos - (cameraData.m_CameraSettings.m_ViewportSize / 2);
        const Utils::Point2DInt renderCoordsTopLeft = cartesianTopLeft.GetFlipped();
        //Utils::Log(std::format("CAMERA: top left: {}", cartesianTopLeft.ToString()));

        //TODO: it seems like this process should proably be much quicker and maybe we should optimize
        //by first checking if the area is smaller and convering those automatically with empty chars
        Utils::Point2DInt localPos = {};
        Utils::Point2DInt globalPos = {};
        const TextChar* posChar = nullptr;
        bool hasFoundChar = false;
        for (int r = 0; r < newBuffer.GetHeight(); r++)
        {
            for (int c = 0; c < newBuffer.GetWidth(); c++)
            {
                posChar = {};
                hasFoundChar = false;
                localPos = { r, c };
                globalPos = renderCoordsTopLeft + localPos;
                for (int layerIndex = layers.size() - 1; layerIndex >= 0; layerIndex--)
                {
                    //std::cout << std::format("ayer index: {}/{} of r: {} c:{}",
                    //std::to_string(layerIndex), std::to_string(layers.size()), std::to_string(r), std::to_string(c)) << std::endl;

                    const RenderLayer& layer = *layers[layerIndex];

                    //Utils::Log(std::format("Attempting char at: {} of HEGIHT: {} WIDTH: {}",
                    //localPos.ToString(), std::to_string(layer.m_TextBuffer.m_HEIGHT), std::to_string(layer.m_TextBuffer.m_WIDTH)));
                    
                    if (!layer.m_SquaredTextBuffer.IsValidPos(globalPos)) continue;
                   
                    
                    posChar = layer.m_SquaredTextBuffer.GetAt(globalPos);
                    if (posChar == nullptr) continue;
                    if (posChar->m_Char== EMPTY_CHAR_PLACEHOLDER) continue;

                    hasFoundChar = true;
                   /* Utils::Log(std::format("Found valid topmost pos at: {} with char: {} color: {} at layer: {} BUT WHEN DIRECT COLOR: {}", 
                    globalPos.ToString(), Utils::ToString(posChar->m_Char), RaylibUtils::ToString(posChar->m_Color), 
                        std::to_string(layerIndex), layer.m_SquaredTextBuffer.ToString(false)));*/
                    break;
                }

                //Since we may have the camera go out of bounds at times to fit player at desired pos, or
                //we may have smaller scene than camera size, we add invalid pos as empty chars
                if (!hasFoundChar)
                {
                    newBuffer.SetAt(localPos, TextChar{ Color(), EMPTY_CHAR_PLACEHOLDER });
                   /* Utils::Log("OUT OF BOUNDS SET EMPTYx");*/
                }
                else
                {
                    newBuffer.SetAt(localPos, *posChar);
                    //Utils::Log(std::format("Setting the position: {} of buffer with char: {}",
                    //localPos.ToString(), posChar->ToString()));
                }
            }
        }
        //Utils::Log(std::format("Camera collapsing has buffer: {}", newBuffer.ToString(false)));
        return newBuffer;
    }
    
    const TextBuffer* CameraSystem::GetCurrentFrameBuffer() const
    {
        if (m_currentFrameBuffer.has_value()) return &(m_currentFrameBuffer.value());
        else return nullptr;
    }
}
