
//NOT USED
#include "pch.hpp"

#include "PlayerSystem.hpp"
#include "raylib.h"
#include "HelperFunctions.hpp"
#include "CartesianPosition.hpp"

namespace ECS
{
    PlayerSystem::PlayerSystem(TransformSystem& transformSystem) :
       m_transformSystem(transformSystem)
    {

    }

    void PlayerSystem::SystemUpdate(Scene& scene, PlayerData& data, ECS::Entity& entity, const float& deltaTime)
    {
        CartesianPosition currentMove = {};
        if (IsKeyPressed(KEY_DOWN)) currentMove.m_Y++;
        if (IsKeyPressed(KEY_UP)) currentMove.m_Y--;
        if (IsKeyPressed(KEY_RIGHT)) currentMove.m_X++;
        if (IsKeyPressed(KEY_LEFT)) currentMove.m_X--;

       if (MovePlayerFromInput(data, entity, currentMove)) scene.IncreaseFrameDirtyComponentCount();
        /* Utils::Log(std::format("Update end PLAYER component dirty; {}", std::to_string(m_isDirty)));*/
    }

    /// <summary>
    /// Returns true if the player has moved based on the x and y input
    /// </summary>
    /// <param name="data"></param>
    /// <param name="entity"></param>
    /// <param name="moveDelta"></param>
    /// <returns></returns>
    bool PlayerSystem::MovePlayerFromInput(const PlayerData& data, Entity& entity, const CartesianPosition& moveDelta)
    {
        bool isZero = moveDelta.m_X == 0 && moveDelta.m_Y == 0;
        if (isZero) return false;

        Utils::Log("MOVING PLAYER");
        m_transformSystem.SetPosDelta(entity.m_Transform, moveDelta);
        return true;
    }
}

