#pragma once
#include "ComponentData.hpp"
#include "Vec2.hpp"

class PlayerData : public ComponentData
{
private:
	float m_moveSpeed;

public:
	PlayerData();
	PlayerData(const float& moveSpeed);

	const float& GetMoveSpeed() const;
};

