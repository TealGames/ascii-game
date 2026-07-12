#pragma once
#include "ECS/Component/Component.hpp"

namespace Engine::UI
{
	class UIContainer : public ECS::Component
	{
	private:
	public:

	private:
	public:
		UIContainer();

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}
