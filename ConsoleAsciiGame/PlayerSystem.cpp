
//NOT USED
#include "pch.hpp"

#include "PlayerSystem.hpp"
#include "raylib.h"
#include "HelperFunctions.hpp"

namespace ECS
{
    PlayerSystem::PlayerSystem(TransformSystem& transformSystem) :
       m_transformSystem(transformSystem)
    {

    }

    void PlayerSystem::SystemUpdate(Scene& scene, PlayerData& data, ECS::Entity& entity, const float& deltaTime)
    {
        data.m_Dirty = false;

        Utils::Point2DInt currentMove = {};
        if (IsKeyPressed(KEY_DOWN)) currentMove.m_Y++;
        if (IsKeyPressed(KEY_UP)) currentMove.m_Y--;
        if (IsKeyPressed(KEY_RIGHT)) currentMove.m_X++;
        if (IsKeyPressed(KEY_LEFT)) currentMove.m_X--;

        data.m_Dirty = MovePlayerFromInput(data, entity, currentMove);
        /* Utils::Log(std::format("Update end PLAYER component dirty; {}", std::to_string(m_isDirty)));*/
    }

    bool PlayerSystem::MovePlayerFromInput(const PlayerData& data, Entity& entity, const Utils::Point2DInt& moveDelta)
    {
        bool isZero = moveDelta.m_X == 0 && moveDelta.m_Y == 0;
        if (isZero) return false;

        TransformData* maybeTransform = entity.TryGetComponent<TransformData>();
        if (!Utils::Assert(maybeTransform!=nullptr, std::format("Tried to move player entity: {} "
            "by delta: {} but failed to retrieve its transform", 
            std::to_string(entity.m_Id), moveDelta.ToString()))) return false;

        m_transformSystem.SetPosDelta(*maybeTransform, moveDelta);
        return true;
    }
}

