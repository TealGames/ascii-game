#include "pch.hpp"
#include "ECS/Component/Types/World/TestComponent.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine
{
	TestComponent::TestComponent() : Component() {}

	void TestComponent::InitFields()
	{
		m_Fields = {};
	}
	void TestComponent::Serialize(Serialization::Serializer& serializer) const
	{
	}
	void TestComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
	}
	std::string TestComponent::ToString() const
	{
		return std::format("[Test]");
	}
}