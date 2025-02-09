
//NOT USED
#include "pch.hpp"

#include "InputSystem.hpp"
#include "raylib.h"
#include "HelperFunctions.hpp"
#include "CartesianPosition.hpp"
#include "ProfilerTimer.hpp"

namespace ECS
{
    InputSystem::InputSystem()
    {

    }

    void InputSystem::SystemUpdate(Scene& scene, InputData& data, ECS::Entity& entity, const float& deltaTime)
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("InputSystem::SystemUpdate");
#endif 

        CartesianGridPosition currentMove = {};
        if (IsKeyDown(KEY_DOWN)) currentMove.m_Y--;
        if (IsKeyDown(KEY_UP)) currentMove.m_Y++;
        if (IsKeyDown(KEY_RIGHT)) currentMove.m_X++;
        if (IsKeyDown(KEY_LEFT)) currentMove.m_X--;

        if (UpdateData(data, currentMove)) scene.IncreaseFrameDirtyComponentCount();
        /* Log(std::format("Update end PLAYER component dirty; {}", std::to_string(m_isDirty)));*/
    }

    /// <summary>
    /// Returns true if the player has moved based on the x and y input
    /// </summary>
    /// <param name="data"></param>
    /// <param name="entity"></param>
    /// <param name="moveDelta"></param>
    /// <returns></returns>
    bool InputSystem::UpdateData(InputData& data, const CartesianGridPosition& moveDelta)
    {
        //Log("Player move delta: {}", moveDelta.ToString());
        data.SetFrameInput(moveDelta);
        return moveDelta.m_X != 0 || moveDelta.m_Y != 0;
    }
}

