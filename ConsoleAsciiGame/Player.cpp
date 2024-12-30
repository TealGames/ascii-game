#include <format>
#include "Player.hpp"
#include "raylib.h"
#include "HelperFunctions.hpp"

namespace ECS
{
    Player::Player(Transform& transform, const char& playerChar) :
        m_transform(transform), m_PlayerChar(playerChar)
    {

    }

    UpdatePriority Player::GetUpdatePriority() const
    {
        return {MAX_PRIORITY};
    }

    void Player::Start()
    {
        
    }

    void Player::UpdateStart(float deltaTime)
    {
        m_isDirty = false;

        Utils::Point2DInt currentMove = {};
        if (IsKeyPressed(KEY_DOWN)) currentMove.m_Y++;
        if (IsKeyPressed(KEY_UP)) currentMove.m_Y--;
        if (IsKeyPressed(KEY_RIGHT)) currentMove.m_X++;
        if (IsKeyPressed(KEY_LEFT)) currentMove.m_X--;
        
        m_isDirty = MovePlayerFromInput(currentMove);
        Utils::Log(std::format("Update end PLAYER component dirty; {}", std::to_string(m_isDirty)));
    }

    void Player::UpdateEnd(float deltaTime) {}

    bool Player::MovePlayerFromInput(const Utils::Point2DInt& moveDelta)
    {
        bool isZero = moveDelta.m_X == 0 && moveDelta.m_Y == 0;
        if (isZero) return false;

        m_transform.SetPosDelta(moveDelta);
        return true;
    }
}

