
//NOT USED
#include "pch.hpp"
#include "InputSystem.hpp"
#include "raylib.h"
#include "HelperFunctions.hpp"
#include "ProfilerTimer.hpp"

namespace ECS
{
    static const std::string MOVE_COMPOUND_NAME = "move";

    InputSystem::InputSystem(InputManager& inputManager) : m_inputManager(inputManager)
    {
        inputManager.TryAddCompoundInput(MOVE_COMPOUND_NAME,
            { {Direction::Up, KEY_UP}, {Direction::Down, KEY_DOWN},
              {Direction::Left, KEY_LEFT} , {Direction::Right, KEY_RIGHT} });
    }

    void InputSystem::SystemUpdate(Scene& scene, InputData& data, ECS::Entity& entity, const float& deltaTime)
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("InputSystem::SystemUpdate");
#endif 
        std::optional<Utils::Point2DInt> moveInput = m_inputManager.TryGetCompoundInputDown(MOVE_COMPOUND_NAME);
        LogError(std::format("MOVE INPUT COMPOUND: {}", m_inputManager.GetCompoundToString(MOVE_COMPOUND_NAME)));

        if (!Assert(this, moveInput.has_value(), std::format("Tried to update input system "
            "but compound input: {} could not be found", MOVE_COMPOUND_NAME))) return;

        if (UpdateData(data, moveInput.value())) scene.IncreaseFrameDirtyComponentCount();
        /* Log(std::format("Update end PLAYER component dirty; {}", std::to_string(m_isDirty)));*/
    }

    /// <summary>
    /// Returns true if the player has moved based on the x and y input
    /// </summary>
    /// <param name="data"></param>
    /// <param name="entity"></param>
    /// <param name="moveDelta"></param>
    /// <returns></returns>
    bool InputSystem::UpdateData(InputData& data, const Utils::Point2DInt& moveDelta)
    {
        //Log("Player move delta: {}", moveDelta.ToString());
        data.SetFrameInput(moveDelta);
        return moveDelta.m_X != 0 || moveDelta.m_Y != 0;
    }
}

