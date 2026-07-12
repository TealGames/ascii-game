#pragma once
#include "ECS/Component/Types/World/InputComponent.hpp"
#include "Core/Input/InputManager.hpp"

namespace Engine::Scenes { class Scene; }
namespace Engine::Input
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

        void SystemUpdate(Scenes::Scene& scene, InputComponent& component, const float& deltaTime);
    };
}


