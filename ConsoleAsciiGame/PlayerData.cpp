#include "pch.hpp"
#include "PlayerData.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"
#include "PhysicsWorld.hpp"
#include "Entity.hpp"

PlayerData::PlayerData() :
	m_body(nullptr), m_xMoveSpeed(), m_maxJumpHeight(), m_initialJumpSpeed() {}

PlayerData::PlayerData(PhysicsBodyData& bodyData, const float& moveSpeed, const float& maxJumpHeight) :
	m_body(&bodyData), m_xMoveSpeed(std::abs(moveSpeed)), m_maxJumpHeight(maxJumpHeight), m_initialJumpSpeed()
{
	m_initialJumpSpeed = std::sqrt(2 * std::abs(GetBodyMutableSafe().GetGravity()) * maxJumpHeight);
}

void PlayerData::InitFields()
{
	m_Fields = {ComponentField("MoveXSpeed", &m_xMoveSpeed)};
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

float PlayerData::GetVerticalDistanceToGround() const
{
	WorldPosition bottomCenter = m_body->GetAABBWorldPos({ 0.5, 0 });
	bottomCenter.m_Y -= 0.01;
	float distance= m_body->GetPhysicsWorldSafe().Raycast(bottomCenter, { 0, -100 }).m_Displacement.m_Y;

	/*LogError(std::format("Player min: {} max: {} bottom center: {} trans: {} ray result: {}", m_body->GetAABBWorldPos({0,0}).ToString(), 
		m_body->GetAABBWorldPos({1, 1}).ToString(), bottomCenter.ToString(), m_body->GetEntitySafeMutable().m_Transform.m_Pos.ToString(), std::to_string(distance)));*/
	return std::abs(distance);
	//throw std::invalid_argument("FART");
}

PhysicsBodyData& PlayerData::GetBodyMutableSafe()
{
	if (!Assert(this, m_body != nullptr, 
		std::format("Tried to get the physics body mutable from player data but it is NULL")))
		throw std::invalid_argument("Invalid PlayerData PhysicsBody State");

	return *m_body;
}

const Utils::Point2DInt& PlayerData::GetFrameInput() const
{
	return m_currentFrameDirectionalInput;
}

const Utils::Point2DInt& PlayerData::GetLastFrameInput() const
{
	return m_lastFrameDirectionalInput;
}

bool PlayerData::HasInputChanged() const
{
	return m_currentFrameDirectionalInput != m_lastFrameDirectionalInput;
}

void PlayerData::SetFrameInput(const Utils::Point2DInt& input)
{
	SetLastFrameInput(m_currentFrameDirectionalInput);
	m_currentFrameDirectionalInput = input;
}
void PlayerData::SetLastFrameInput(const Utils::Point2DInt& lastFrameInput)
{
	m_lastFrameDirectionalInput = lastFrameInput;
}

Utils::Point2DInt PlayerData::GetInputDelta() const
{
	return m_currentFrameDirectionalInput - m_lastFrameDirectionalInput;
}