#include "pch.hpp"
#include "ECS/Component/Types/World/TriggerComponent.hpp"
#include "Utils/Debug.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace ThisNamespace = Engine::World;
namespace Engine::World
{
	std::string ToString(const TriggerType& trigger)
	{
		if (trigger == TriggerType::Default) return "Default";
		else if (trigger == TriggerType::Scene) return "Scene";
		else
		{
			LogError("Tried to stringify trigger type but type actions could not be found");
			return "";
		}
	}

	TriggerComponent::TriggerComponent() : TriggerComponent(nullptr) {}
	TriggerComponent::TriggerComponent(const CollisionBoxComponent* collider) :
		Component(), m_collider(collider), m_triggerType(TriggerType::Default), m_OnEnter(), m_OnExit() {}

	const CollisionBoxComponent& TriggerComponent::GetCollisionBox() const
	{
		ENGINE_ASSERT(m_collider != nullptr, "Tried to get collider but it is NULL");
		return *m_collider;
	}

	//std::vector<std::string> TriggerComponent::GetDependencyFlags() const
	//{
	//	return { ::Utils::GetTypeName<CollisionBoxComponent>()};
	//}
	void TriggerComponent::InitFields()
	{
		m_Fields = {};
	}
	void TriggerComponent::Serialize(Serialization::Serializer& serializer) const
	{
	}
	void TriggerComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
	}

	std::string TriggerComponent::ToString() const
	{
		return std::format("[Trigger Type:{}]", ThisNamespace::ToString(m_triggerType));
	}
}