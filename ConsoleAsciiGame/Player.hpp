#pragma once
#include "Point2DInt.hpp"
#include "Component.hpp"
#include "Transform.hpp" 

namespace ECS
{
    class Player : public Component
    {
    private:
        Transform& m_transform;
    public:
        const char m_PlayerChar;

    private:
        bool MovePlayerFromInput(const Utils::Point2DInt& moveDelta);

    public:
        Player(Transform& transform, const char& playerChar);

        UpdatePriority GetUpdatePriority() const override;

        void Start() override;
        void UpdateStart(float deltaTime) override;
        void UpdateEnd(float deltaTime) override;
    };
}


