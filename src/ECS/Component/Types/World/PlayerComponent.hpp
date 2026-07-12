#pragma once
#include "ECS/Component/Component.hpp"
#include "ECS/Component/Types/World/PhysicsBodyComponent.hpp"
#include "Core/Primitives/Vector.hpp"

namespace Engine::Physics { class PhysicsBodyComponent; }
namespace Engine::Player
{
	class PlayerSystem;
	class PlayerComponent : public Engine::ECS::Component
	{
	private:
		Physics::PhysicsBodyComponent* m_body;

		float m_xMoveSpeed;
		float m_initialJumpSpeed;
		float m_maxJumpHeight;

		Vec2Int m_currentFrameDirectionalInput;
		Vec2Int m_lastFrameDirectionalInput;

	public:
		friend class PlayerSystem;

	private:
		float CalculateInitialJumpSpeed() const;

	private:
		PlayerComponent(Physics::PhysicsBodyComponent* body, const float& moveSpeed, const float& maxJumpHeight);

		void TrySetInitialJumpSpeed();

	public:
		PlayerComponent();
		//TODO: what if the wrong boddy is provided meaning one that does not share the same entity as this
		PlayerComponent(Physics::PhysicsBodyComponent& bodyData, const float& moveSpeed, const float& maxJumpHeight);

		float GetMoveSpeed() const;
		float GetInitialJumpSpeed() const;
		bool GetIsGrounded() const;

		/// <summary>
		/// Will send out a raycast until a ground is found (or ray ends)
		/// Note: this function is expensive so it should be used sparingly
		/// </summary>
		/// <returns></returns>
		float GetVerticalDistanceToGround() const;

		Physics::PhysicsBodyComponent& GetBodyMutableSafe();
		const Physics::PhysicsBodyComponent& GetBodySafe() const;

		Vec2Int GetFrameInput() const;
		Vec2Int GetLastFrameInput() const;
		Vec2Int GetInputDelta() const;
		bool HasInputChanged() const;

		void SetFrameInput(const Vec2Int& input);
		void SetLastFrameInput(const Vec2Int& lastFrameInput);

		//std::vector<std::string> GetDependencyFlags() const override;
		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}


