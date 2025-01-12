#include "pch.hpp"
#include "Entity.hpp"
#include "CameraSystem.hpp"
#include "Point2DInt.hpp"
#include "Updateable.hpp"
#include "Component.hpp"
#include "Globals.hpp"
#include "SceneManager.hpp"
//#include "GameRenderer.hpp"
#include "CartesianPosition.hpp"
#include "Array2DPosition.hpp"
#include "PositionConversions.hpp"
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
            //Utils::Log("NO camera render update");
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
        m_transformSystem.SetPos(mainCamera.m_Transform,
            cameraData.m_CameraSettings.m_FollowTarget->m_Transform.m_Pos);
    }

    void CameraSystem::CollapseLayersWithinViewport(const Scene& scene, CameraData& cameraData, ECS::Entity& mainCamera)
    {
        //TODO: this is cuainsg some performance rpboelms
        m_currentFrameBuffer = TextBuffer{ cameraData.m_CameraSettings.m_ViewportSize.m_X, cameraData.m_CameraSettings.m_ViewportSize.m_Y, TextChar{} };

        TextBuffer& newBuffer = m_currentFrameBuffer.value();
        /*TextBuffer newBuffer = TextBuffer(cameraData.m_CameraSettings.m_ViewportSize.m_X, cameraData.m_CameraSettings.m_ViewportSize.m_Y, TextChar{});*/
        const std::vector<const RenderLayer*> layers = scene.GetAllLayers();
       /* Utils::Log(std::format("ONE LAYER IN CAMERA: {}", layers[0]->ToString()));
        Utils::Log(std::format("SECOND LAYER IN CAMERA: {}", layers[1]->ToString()));*/

        const CartesianPosition cartesianTopLeft = mainCamera.m_Transform.m_Pos - (cameraData.m_CameraSettings.m_ViewportSize / 2);
        const Array2DPosition renderCoordsTopLeft = Conversions::CartesianToArray(cartesianTopLeft);
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

                    /*std::cout << std::format("Layer index: {}/{} of r: {} c:{} GLOBAL POS: {} valid: {}",
                    std::to_string(layerIndex), std::to_string(layers.size()), std::to_string(r), 
                        std::to_string(c), globalPos.ToString(), layer.m_SquaredTextBuffer.IsValidPos(globalPos)) << std::endl;*/

                    //Utils::Log(std::format("Attempting char at: {} of HEGIHT: {} WIDTH: {}",
                    //localPos.ToString(), std::to_string(layer.m_TextBuffer.m_HEIGHT), std::to_string(layer.m_TextBuffer.m_WIDTH)));
                    
                    if (!layer.m_SquaredTextBuffer.IsValidPos(globalPos)) continue;
                   
                    
                    const TextChar posChar = layer.m_SquaredTextBuffer.GetAtUnsafe(globalPos);
                    //if (posChar == nullptr) continue;
                    if (posChar.m_Char== EMPTY_CHAR_PLACEHOLDER) continue;

                    hasFoundChar = true;
                    newBuffer.SetAt(localPos, posChar);
                    /*Utils::Log(std::format("Setting the position: {} of buffer with char: {}",
                        localPos.ToString(), posChar.ToString()));*/
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
                    //Utils::Log("OUT OF BOUNDS SET EMPTYx");
                }
            }
        }
        //Utils::Log(std::format("Camera collapsing has buffer: {}", newBuffer.ToString(false)));
        //return newBuffer;
    }
    
    const TextBuffer* CameraSystem::GetCurrentFrameBuffer() const
    {
        if (m_currentFrameBuffer.has_value()) return &(m_currentFrameBuffer.value());
        else return nullptr;
    }
}
