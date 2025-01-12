#pragma once
#include <vector>
#include "SingleBodySystem.hpp"
#include "Point2DInt.hpp"
#include "Component.hpp"
#include "PlayerData.hpp"
#include "TransformSystem.hpp"
#include "EntityMapper.hpp"
#include "CartesianPosition.hpp"

namespace ECS
{
    class PlayerSystem : public SingleBodySystem<PlayerData>
    {
    private:
        TransformSystem& m_transformSystem;

    public:

    private:
        bool MovePlayerFromInput(const PlayerData& data, Entity& entity, const CartesianPosition& moveDelta);

    public:
        PlayerSystem(TransformSystem& transformSystem);
        ~PlayerSystem() = default;

        void SystemUpdate(Scene& scene, PlayerData& component, ECS::Entity& entity, const float& deltaTime) override;
    };
}


