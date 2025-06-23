#pragma once
#include <vector>
#include "SingleBodySystem.hpp"
#include "InputData.hpp"
#include "InputManager.hpp"

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


