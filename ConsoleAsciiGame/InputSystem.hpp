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
        InputManager& m_inputManager;
    public:

    private:
        bool UpdateData(InputData& data, const Utils::Point2DInt& moveDelta);

    public:
        InputSystem(InputManager& inputManager);
        ~InputSystem() = default;

        void SystemUpdate(Scene& scene, InputData& component, ECS::Entity& entity, const float& deltaTime) override;
    };
}


