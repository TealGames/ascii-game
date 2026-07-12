#include "pch.hpp"
#include "ECS/Component/Types/World/PlayerComponent.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Utils/Debug.hpp"
#include "Core/Physics/PhysicsWorld.hpp"
#include "Core/Serialization/Serializer.hpp"
#include "Core/Serialization/SerializationUtils.hpp"

namespace Engine::Player
{
	PlayerComponent::PlayerComponent() :
		PlayerComponent(nullptr, 0, 0) {}

	PlayerComponent::PlayerComponent(Physics::PhysicsBodyComponent* body, const float& moveSpeed, const float& maxJumpHeight) :
		Component(),
		m_body(body), m_xMoveSpeed(std::abs(moveSpeed)), m_maxJumpHeight(maxJumpHeight), m_initialJumpSpeed()
	{
		//if (m_maxJumpHeight>0) m_initialJumpSpeed = CalculateInitialJumpSpeed();
		TrySetInitialJumpSpeed();
	}

	PlayerComponent::PlayerComponent(Physics::PhysicsBodyComponent& bodyData, const float& moveSpeed, const float& maxJumpHeight) :
		PlayerComponent(&bodyData, moveSpeed, maxJumpHeight) {}

	void PlayerComponent::InitFields()
	{
		m_Fields = { Engine::ECS::ComponentField("MoveXSpeed", &m_xMoveSpeed) };
	}

	void PlayerComponent::Serialize(Serialization::Serializer& serializer) const
	{
		serializer.AddProperty("MoveSpeed", m_xMoveSpeed);
		serializer.AddProperty("JumpHeight", m_maxJumpHeight);
		serializer.AddProperty("Body", Serialization::TrySerializeComponent(m_body, true));
	}
	void PlayerComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
		deserializer.GetProperty("MoveSpeed", &m_xMoveSpeed);
		deserializer.GetProperty("JumpHeight", &m_maxJumpHeight);

		std::optional<Serialization::SerializedComponent> maybeSerializedBody = std::nullopt;
		deserializer.GetProperty("Body", &maybeSerializedBody);
		m_body = Serialization::TryDeserializeComponent<Physics::PhysicsBodyComponent>(maybeSerializedBody, GetEntityMutable(), true);

		TrySetInitialJumpSpeed();
	}

	void PlayerComponent::TrySetInitialJumpSpeed()
	{
		if (m_body == nullptr || m_maxJumpHeight < 0) return;

		m_initialJumpSpeed = CalculateInitialJumpSpeed();
		//Assert(false, std::format("Init speed:{}", std::to_string(m_initialJumpSpeed)));
	}

	float PlayerComponent::GetMoveSpeed() const
	{
		return m_xMoveSpeed;
	}
	float PlayerComponent::CalculateInitialJumpSpeed() const
	{
		return std::sqrt(2 * std::abs(GetBodySafe().GetGravity()) * m_maxJumpHeight);
	}
	float PlayerComponent::GetInitialJumpSpeed() const
	{
		return m_initialJumpSpeed;
	}
	bool PlayerComponent::GetIsGrounded() const
	{
		return !m_body->IsExperiencingGravity();
	}

	float PlayerComponent::GetVerticalDistanceToGround() const
	{
		//WorldPosition bottomCenter = m_body->GetAABBWorldPos({ 0.5, 0 });
		WorldPosition2D bottomCenter = m_body->GetCollisionBox().GetAABBWorldPos({ 0.5, 0 });
		bottomCenter.m_Y -= 0.01;
		float distance = m_body->GetPhysicsWorldSafe().Raycast2D(bottomCenter, { 0, -100 }).m_Displacement.m_Y;

		/*LogError(std::format("Player min: {} max: {} bottom center: {} trans: {} ray result: {}", m_body->GetAABBWorldPos({0,0}).ToString(),
			m_body->GetAABBWorldPos({1, 1}).ToString(), bottomCenter.ToString(), m_body->GetEntitySafeMutable().m_Transform.m_Pos.ToString(), std::to_string(distance)));*/
		return std::abs(distance);
		//throw std::invalid_argument("FART");
	}

	Physics::PhysicsBodyComponent& PlayerComponent::GetBodyMutableSafe()
	{
		ENGINE_ASSERT(m_body != nullptr,
			"Tried to get the physics body MUTABLE from player data but it is NULL");

		return *m_body;
	}
	const Physics::PhysicsBodyComponent& PlayerComponent::GetBodySafe() const
	{
		ENGINE_ASSERT(m_body != nullptr,
			"Tried to get the physics body from player data but it is NULL");

		return *m_body;
	}

	Vec2Int PlayerComponent::GetFrameInput() const
	{
		return m_currentFrameDirectionalInput;
	}

	Vec2Int PlayerComponent::GetLastFrameInput() const
	{
		return m_lastFrameDirectionalInput;
	}

	bool PlayerComponent::HasInputChanged() const
	{
		return m_currentFrameDirectionalInput != m_lastFrameDirectionalInput;
	}

	void PlayerComponent::SetFrameInput(const Vec2Int& input)
	{
		SetLastFrameInput(m_currentFrameDirectionalInput);
		m_currentFrameDirectionalInput = input;
	}
	void PlayerComponent::SetLastFrameInput(const Vec2Int& lastFrameInput)
	{
		m_lastFrameDirectionalInput = lastFrameInput;
	}

	Vec2Int PlayerComponent::GetInputDelta() const
	{
		return m_currentFrameDirectionalInput - m_lastFrameDirectionalInput;
	}

	std::string PlayerComponent::ToString() const
	{
		return std::format("[PlayerComponent MoveSpeed:{} JumpHeight:{} Body:{}]",
			std::to_string(m_xMoveSpeed), std::to_string(m_maxJumpHeight), m_body != nullptr ? m_body->ToString() : "NULL");
	}
}
