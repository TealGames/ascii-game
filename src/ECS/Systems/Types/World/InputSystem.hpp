#pragma once
#include <vector>
#include "ECS/Systems/SingleBodySystem.hpp"
#include "ECS/Component/Types/World/InputData.hpp"
#include "Core/Input/InputManager.hpp"

namespace ECS
{
    class InputSystem : public SingleBodySystem<InputData>
    {
    private:
        Input::InputManager& m_inputManager;
    public:

    private:

    public:
        InputSystem(Input::InputManager& inputManager);
        ~InputSystem() = default;

        void SystemUpdate(Scene& scene, InputData& component, const float& deltaTime) override;
    };
}


