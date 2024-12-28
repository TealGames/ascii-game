#include "Camera.hpp"
#include "Point2DInt.hpp"
#include "Entity.hpp"
#include "Updateable.hpp"
#include "Component.hpp"
#include "Globals.hpp"
#include "SceneManager.hpp"
#include "GameRenderer.hpp"

namespace ECS
{
    static constexpr bool CACHE_LAST_BUFFER = true;

	Camera::Camera(Transform& transform, const SceneManagement::SceneManager& sceneManager,
        ECS::Entity& followTarget, const Utils::Point2DInt& viewportSize) :
		m_sceneManager(sceneManager), m_transform(transform), m_followTarget(&followTarget), 
        m_viewportSize(viewportSize), m_lastFrameBuffer(std::nullopt)
	{
	}

	/*Camera::Camera(Transform& transform, const Utils::Point2DInt& viewportSize) :
		m_transform(transform), m_followTarget(nullptr), m_size(viewportSize)
	{
	}*/

    //TODO: this should be modified to have a follow delay, lookeahead blocks, etc to be more dynamic
    void Camera::UpdateCameraPosition()
    {
        m_transform.SetPos(m_followTarget->m_Transform.m_Pos);
    }

	UpdatePriority Camera::GetUpdatePriority() const
	{
		return { MIN_PRIORITY };
	}

	void Camera::Start() {}
	void Camera::UpdateStart(float deltaTime) 
    {
        //TODO: another condition to optimize could be if scene is not marked as dirty (meaning no changes occured)
        //we can just not update
        if (CACHE_LAST_BUFFER && !m_followTarget->m_Transform.HasMovedThisFrame()
            && m_lastFrameBuffer.has_value())
        {
            Utils::Log("Saving camera render");
            Rendering::RenderBuffer(m_lastFrameBuffer.value());
            return;
        }
        UpdateCameraPosition();
        //Rendering::RenderBuffer(TextBuffer(5, 5, TextChar(BLUE, 'V')));
        //m_lastFrameBuffer = TextBuffer(5, 5, TextChar(BLUE, 'V'));
        //m_lastFrameBuffer = CollapseLayersWithinViewport();
       // return;

        TextBuffer collapsedBuffer = CollapseLayersWithinViewport();
        Rendering::RenderBuffer(collapsedBuffer);
        m_lastFrameBuffer = collapsedBuffer;
    }

	void Camera::UpdateEnd(float deltaTime) {}

    TextBuffer Camera::CollapseLayersWithinViewport() const
    {
        TextBuffer newBuffer = TextBuffer(m_viewportSize.m_X, m_viewportSize.m_Y, TextChar{});
        const std::vector<const RenderLayer*> layers = m_sceneManager.GetActiveScene()->GetLayers();

        const Utils::Point2DInt cartesianTopLeft = m_transform.m_Pos - (m_viewportSize / 2);
        const Utils::Point2DInt renderCoordsTopLeft = cartesianTopLeft.GetFlipped();
        /*Utils::Log(std::format("Collapse layers total: {} all layers: {} player pos: {} camera size: {} caresian tl: {} row col top left: {}", 
            std::to_string(layers.size()), m_sceneManager.GetActiveScene()->ToStringLayers(),
            m_followTarget->m_Transform.m_Pos.ToString(), m_viewportSize.ToString(),
            cartesianTopLeft.ToString(), renderCoordsTopLeft.ToString()));*/

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
                    /*std::cout << std::format("ayer index: {}/{} of r: {} c:{}",
                        std::to_string(layerIndex), std::to_string(layers.size()), std::to_string(r), std::to_string(c)) << std::endl;*/

                    const RenderLayer& layer = *layers[layerIndex];

                    /* Utils::Log(std::format("Attempting char at: {} of HEGIHT: {} WIDTH: {}",
                         localPos.ToString(), std::to_string(layer.m_TextBuffer.m_HEIGHT), std::to_string(layer.m_TextBuffer.m_WIDTH)));*/
                    
                    if (!layer.m_SquaredTextBuffer.IsValidPos(globalPos)) continue;
                   
                    
                    posChar = layer.m_SquaredTextBuffer.GetAt(globalPos);
                    if (posChar == nullptr) continue;
                    if (posChar->m_Char== EMPTY_CHAR_PLACEHOLDER) continue;

                    hasFoundChar = true;
                    /*Utils::Log(std::format("Found valid topmost pos at: {} with char: {} at layer: {}", 
                        globalPos.ToString(), Utils::ToString(posChar->m_Char), std::to_string(layerIndex)));*/
                    break;
                }

                //Since we may have the camera go out of bounds at times to fit player at desired pos, or
                //we may have smaller scene than camera size, we add invalid pos as empty chars
                if (!hasFoundChar) newBuffer.SetAt(localPos, TextChar{ Color(), EMPTY_CHAR_PLACEHOLDER });
                else
                {
                    newBuffer.SetAt(localPos, *posChar);
                   /* Utils::Log(std::format("Setting the position: {} of buffer with char: {}",
                        localPos.ToString(), posChar->ToString()));*/
                }
            }
        }
        //Utils::Log(std::format("Camera collapsing has buffer: {}", newBuffer.ToString()));
        return newBuffer;
    }
}
