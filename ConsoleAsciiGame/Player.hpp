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
    public:
        Player(Transform& transform, const char& playerChar);

        void Start() override;
        void Update(float deltaTime) override;
    };
}


