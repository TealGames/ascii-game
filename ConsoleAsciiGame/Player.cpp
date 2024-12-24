#include "Player.hpp"
#include "raylib.h"

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

        if (IsKeyPressed(KEY_DOWN))
        {
            m_transform.SetDeltaY(1);
        }
        else if (IsKeyPressed(KEY_UP))
        {
            m_transform.SetDeltaY(-1);
        }
        else if (IsKeyPressed(KEY_RIGHT))
        {
            m_transform.SetDeltaX(1);
        }
        else if (IsKeyPressed(KEY_LEFT))
        {
            m_transform.SetDeltaX(-1);
        }
    }

    void Player::UpdateEnd(float deltaTime) {}
}

