#include "pch.hpp"
#include "PlayerData.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"
#include "PhysicsWorld.hpp"
#include "JsonSerializers.hpp"

PlayerData::PlayerData() :
	PlayerData(nullptr, 0, 0) {}

PlayerData::PlayerData(const Json& json) : PlayerData()
{
	Deserialize(json);
}
PlayerData::PlayerData(PhysicsBodyData* body, const float& moveSpeed, const float& maxJumpHeight) : 
	ComponentData(),
	m_body(body), m_xMoveSpeed(std::abs(moveSpeed)), m_maxJumpHeight(maxJumpHeight), m_initialJumpSpeed()
{
	//if (m_maxJumpHeight>0) m_initialJumpSpeed = CalculateInitialJumpSpeed();
	TrySetInitialJumpSpeed();
}

PlayerData::PlayerData(PhysicsBodyData& bodyData, const float& moveSpeed, const float& maxJumpHeight) :
	PlayerData(&bodyData, moveSpeed, maxJumpHeight) {}

void PlayerData::InitFields()
{
	m_Fields = {ComponentField("MoveXSpeed", &m_xMoveSpeed)};
}
std::vector<std::string> PlayerData::GetDependencyFlags() const
{
	return {Utils::GetTypeName<PhysicsBodyData>()};
}

void PlayerData::TrySetInitialJumpSpeed()
{
	if (m_body == nullptr || m_maxJumpHeight < 0) return;

	m_initialJumpSpeed = CalculateInitialJumpSpeed();
	//Assert(false, std::format("Init speed:{}", std::to_string(m_initialJumpSpeed)));
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
	//WorldPosition bottomCenter = m_body->GetAABBWorldPos({ 0.5, 0 });
	WorldPosition bottomCenter = m_body->GetCollisionBox().GetAABBWorldPos({ 0.5, 0 });
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

void PlayerData::Deserialize(const Json& json)
{
	m_xMoveSpeed = json.at("MoveSpeed").get<float>();
	m_maxJumpHeight = json.at("JumpHeight").get<float>();

	m_body = TryDeserializeComponent<PhysicsBodyData>(json.at("Body"), GetEntitySafeMutable());
	TrySetInitialJumpSpeed();
}
Json PlayerData::Serialize()
{
	//Assert(false, std::format("Serializign player data comp entity:{} scene:{}", m_body->GetEntitySafe().GetName(), m_body->GetEntitySafe().GetSceneName()));
	return { {"MoveSpeed", m_xMoveSpeed}, {"JumpHeight", m_maxJumpHeight}, {"Body", TrySerializeComponent<PhysicsBodyData>(m_body)}};
}

std::string PlayerData::ToString() const
{
	return std::format("[PlayerData MoveSpeed:{} JumpHeight:{} Body:{}]", 
		std::to_string(m_xMoveSpeed), std::to_string(m_maxJumpHeight), m_body!=nullptr? m_body->ToString(): "NULL");
}