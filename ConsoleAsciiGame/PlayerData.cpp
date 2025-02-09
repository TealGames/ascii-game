#include "pch.hpp"
#include "PlayerData.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"

PlayerData::PlayerData() :
	m_body(nullptr), m_xMoveSpeed(), m_maxJumpHeight(), m_initialJumpSpeed() {}

PlayerData::PlayerData(PhysicsBodyData& bodyData, const float& moveSpeed, const float& maxJumpHeight) :
	m_body(&bodyData), m_xMoveSpeed(std::abs(moveSpeed)), m_maxJumpHeight(maxJumpHeight), m_initialJumpSpeed()
{
	m_initialJumpSpeed = std::sqrt(2 * std::abs(GetBodyMutableSafe().GetGravity()) * maxJumpHeight);
}

const float& PlayerData::GetMoveSpeed() const
{
	return m_xMoveSpeed;
}
const float& PlayerData::GetInitialJumpSpeed() const
{
	return m_initialJumpSpeed;
}
const bool& PlayerData::GetIsGrounded() const
{
	return !m_body->IsExperiencingGravity();
}

PhysicsBodyData& PlayerData::GetBodyMutableSafe()
{
	if (!Assert(this, m_body != nullptr, 
		std::format("Tried to get the physics body mutable from player data but it is NULL")))
		throw std::invalid_argument("Invalid PlayerData PhysicsBody State");

	return *m_body;
}