#include "pch.hpp"
#include "PlayerData.hpp"

PlayerData::PlayerData() : PlayerData(0) {}
PlayerData::PlayerData(const float& moveSpeed) : m_moveSpeed(moveSpeed) {}

const float& PlayerData::GetMoveSpeed() const
{
	return m_moveSpeed;
}