#pragma once
#include "ECS/Component/Component.hpp"

namespace Engine
{
	class TestComponent : public ECS::Component
	{
	private:
	public:
		TestComponent();

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}