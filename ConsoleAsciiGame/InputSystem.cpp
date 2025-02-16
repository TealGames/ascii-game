
//NOT USED
#include "pch.hpp"
#include "InputSystem.hpp"
#include "raylib.h"
#include "HelperFunctions.hpp"

#ifdef ENABLE_PROFILER
#include "ProfilerTimer.hpp"
#endif 

namespace ECS
{
    static const std::string MOVE_COMPOUND_NAME = "move";

    InputSystem::InputSystem(Input::InputManager& inputManager) 
        : m_inputManager(inputManager)
    {
    }

    void InputSystem::SystemUpdate(Scene& scene, InputData& data, ECS::Entity& entity, const float& deltaTime)
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("InputSystem::SystemUpdate");
#endif 

        //const Input::InputProfile* inputProfile = m_inputManager.TryGetProfile(MAIN_INPUT_PROFILE_NAME);
        //if (!Assert(this, inputProfile != nullptr, std::format("Tried to update input for player "
        //    "but the input profile: '{}' was not found", MAIN_INPUT_PROFILE_NAME)))
        //    return;

        //const Input::CompoundInput* moveCompound = inputProfile->TryGetCompoundInputAction(MAIN_INPUT_PROFILE_MOVE_ACTION);
        //if (!Assert(this, moveCompound != nullptr, std::format("Tried to update input for player "
        //    "but the move compound: '{}' was not found in input profile: {}",
        //    MAIN_INPUT_PROFILE_MOVE_ACTION, MAIN_INPUT_PROFILE_NAME)))
        //    return;

        //Utils::Point2DInt moveInput = moveCompound->GetCompoundInputDown();
        //if (UpdateData(data, moveInput)) scene.IncreaseFrameDirtyComponentCount();
        ///* Log(std::format("Update end PLAYER component dirty; {}", std::to_string(m_isDirty)));*/
    }

    /// <summary>
    /// Returns true if the player has moved based on the x and y input
    /// </summary>
    /// <param name="data"></param>
    /// <param name="entity"></param>
    /// <param name="moveDelta"></param>
    /// <returns></returns>
    //bool InputSystem::UpdateData(InputData& data, const Utils::Point2DInt& moveDelta)
    //{
    //    //Log("Player move delta: {}", moveDelta.ToString());
    //    data.SetFrameInput(moveDelta);
    //    return moveDelta.m_X != 0 || moveDelta.m_Y != 0;
    //}
}

