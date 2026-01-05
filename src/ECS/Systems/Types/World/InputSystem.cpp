#include "pch.hpp"
#include "ECS/Systems/Types/World/InputSystem.hpp"
#include "Utils/HelperFunctions.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 

namespace ECS
{
    static const std::string MOVE_COMPOUND_NAME = "move";

    InputSystem::InputSystem(Input::InputManager& inputManager) 
        : m_inputManager(inputManager)
    {
    }

    void InputSystem::SystemUpdate(Scene& scene, InputData& data, const float& deltaTime)
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("InputSystem::SystemUpdate");
#endif 

        //const Input::InputProfile* inputProfile = m_inputManager.TryGetProfile(MAIN_INPUT_PROFILE_NAME);
        //if (!Assert(inputProfile != nullptr, std::format("Tried to update input for player "
        //    "but the input profile: '{}' was not found", MAIN_INPUT_PROFILE_NAME)))
        //    return;

        //const Input::CompoundInput* moveCompound = inputProfile->TryGetCompoundInputAction(MAIN_INPUT_PROFILE_MOVE_ACTION);
        //if (!Assert(moveCompound != nullptr, std::format("Tried to update input for player "
        //    "but the move compound: '{}' was not found in input profile: {}",
        //    MAIN_INPUT_PROFILE_MOVE_ACTION, MAIN_INPUT_PROFILE_NAME)))
        //    return;

        //Utils::Point2DInt moveInput = moveCompound->GetCompoundInputDown();
        //if (UpdateData(data, moveInput)) scene.IncreaseFrameDirtyComponentCount();
        ///* Log(std::format("Update end PLAYER component dirty; {}", std::to_string(m_isDirty)));*/
    }
}

