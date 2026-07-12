#pragma once
#include "ECS/Component/Component.hpp"
#include <functional>
#include "Utils/Data/Event.hpp"
#include "ECS/Component/Types/World/CollisionBoxComponent.hpp"

namespace Engine::World
{
	using CollisionBoxComponent = Engine::Physics::CollisionBoxComponent;
	/// <summary>
	/// The type of trigger that will determine any preset trigger functions 
	/// that are triggered 
	/// </summary>
	enum class TriggerType
	{
		/// <summary>
		/// 
		/// </summary>
		Default,
		Scene,
	};
	std::string ToString(const TriggerType& trigger);

	class TriggerComponent : public Engine::ECS::Component
	{
	private:
		const CollisionBoxComponent* m_collider;
		TriggerType m_triggerType;
	public:
		friend class TriggerSystem;

		Event<void, const CollisionBoxComponent*> m_OnEnter;
		Event<void, const CollisionBoxComponent*> m_OnExit;

	public:
		TriggerComponent();
		TriggerComponent(const CollisionBoxComponent* collider);

		const CollisionBoxComponent& GetCollisionBox() const;

		//std::vector<std::string> GetDependencyFlags() const override;
		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}


