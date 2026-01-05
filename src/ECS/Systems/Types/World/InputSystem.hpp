#pragma once
#include "ECS/Component/Types/World/InputData.hpp"
#include "Core/Input/InputManager.hpp"

class Scene;
namespace ECS
{
    class InputSystem
    {
    private:
        Input::InputManager& m_inputManager;
    public:

    private:

    public:
        InputSystem(Input::InputManager& inputManager);
        ~InputSystem() = default;

        void SystemUpdate(Scene& scene, InputData& component, const float& deltaTime);
    };
}


