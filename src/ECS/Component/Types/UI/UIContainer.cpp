#include "pch.hpp"
#include "ECS/Component/Types/UI/UIContainer.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::UI
{
	UIContainer::UIContainer() : Component() {}

	void UIContainer::InitFields()
	{
		m_Fields = {};
	}
	void UIContainer::Serialize(Serialization::Serializer& serializer) const
	{
	}
	void UIContainer::Deserialize(Serialization::Deserializer& deserializer)
	{
	}
	std::string UIContainer::ToString() const
	{
		return std::format("[ContainerGUI]");
	}
}

