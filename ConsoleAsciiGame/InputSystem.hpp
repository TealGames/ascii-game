#pragma once
#include <vector>
#include "SingleBodySystem.hpp"
#include "Point2DInt.hpp"
#include "Component.hpp"
#include "InputData.hpp"
#include "TransformSystem.hpp"
#include "EntityMapper.hpp"

namespace ECS
{
    class InputSystem : public SingleBodySystem<InputData>
    {
    private:

    public:

    private:
        bool UpdateData(InputData& data, const Utils::Point2DInt& moveDelta);

    public:
        InputSystem();
        ~InputSystem() = default;

        void SystemUpdate(Scene& scene, InputData& component, ECS::Entity& entity, const float& deltaTime) override;
    };
}


