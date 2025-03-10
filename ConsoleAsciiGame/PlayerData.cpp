#include "pch.hpp"
#include "PlayerData.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"
#include "PhysicsWorld.hpp"

PlayerData::PlayerData() :
	m_body(nullptr), m_xMoveSpeed(), m_maxJumpHeight(), m_initialJumpSpeed() {}

PlayerData::PlayerData(const Json& json) : PlayerData()
{
	Deserialize(json);
}
PlayerData::PlayerData(PhysicsBodyData& bodyData, const float& moveSpeed, const float& maxJumpHeight) :
	m_body(&bodyData), m_xMoveSpeed(std::abs(moveSpeed)), m_maxJumpHeight(maxJumpHeight), m_initialJumpSpeed()
{
	m_initialJumpSpeed = CalculateInitialJumpSpeed();
}

void PlayerData::InitFields()
{
	m_Fields = {ComponentField("MoveXSpeed", &m_xMoveSpeed)};
}

const float& PlayerData::GetMoveSpeed() const
{
	return m_xMoveSpeed;
}
float PlayerData::CalculateInitialJumpSpeed() const
{
	return std::sqrt(2 * std::abs(GetBodySafe().GetGravity()) * m_maxJumpHeight);
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
		std::format("Tried to get the physics body MUTABLE from player data but it is NULL")))
		throw std::invalid_argument("Invalid PlayerData PhysicsBody State");

	return *m_body;
}
const PhysicsBodyData& PlayerData::GetBodySafe() const
{
	if (!Assert(this, m_body != nullptr,
		std::format("Tried to get the physics body from player data but it is NULL")))
		throw std::invalid_argument("Invalid PlayerData PhysicsBody State");

	return *m_body;
}

const Vec2Int& PlayerData::GetFrameInput() const
{
	return m_currentFrameDirectionalInput;
}

const Vec2Int& PlayerData::GetLastFrameInput() const
{
	return m_lastFrameDirectionalInput;
}

bool PlayerData::HasInputChanged() const
{
	return m_currentFrameDirectionalInput != m_lastFrameDirectionalInput;
}

void PlayerData::SetFrameInput(const Vec2Int& input)
{
	SetLastFrameInput(m_currentFrameDirectionalInput);
	m_currentFrameDirectionalInput = input;
}
void PlayerData::SetLastFrameInput(const Vec2Int& lastFrameInput)
{
	m_lastFrameDirectionalInput = lastFrameInput;
}

Vec2Int PlayerData::GetInputDelta() const
{
	return m_currentFrameDirectionalInput - m_lastFrameDirectionalInput;
}

PlayerData& PlayerData::Deserialize(const Json& json)
{
	m_xMoveSpeed = json.at("MoveSpeed").get<float>();
	m_maxJumpHeight = json.at("JumpHeight").get<float>();
	m_initialJumpSpeed = CalculateInitialJumpSpeed();
	return *this;
}
Json PlayerData::Serialize(const PlayerData& component)
{
	return { {"MoveSpeed", m_xMoveSpeed}, {"JumpHeight", m_maxJumpHeight}};
}

std::string PlayerData::ToString() const
{
	return std::format("[PlayerData MoveSpeed:{} JumpHeight:{}]", 
		std::to_string(m_xMoveSpeed), std::to_string(m_maxJumpHeight));
}