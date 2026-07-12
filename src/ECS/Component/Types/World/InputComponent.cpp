//NOT USED
#include "pch.hpp"
#include "ECS/Component/Types/World/InputComponent.hpp"	
#include "Utils/Debug.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::Input
{
	InputComponent::InputComponent() : Component(),
		m_actions() {}

	void InputComponent::AddAction(const Input::InputAction& action)
	{
		if (!Assert(m_actions.find(action.m_Name) != m_actions.end(), "Tried to add the input action: {}"
			" to input data of entity: '{}', but that action already exists for this input data!",
			action.m_Name, GetEntity().m_Name))
			return;

		m_actions.emplace(action.m_Name, action);
	}
	const Input::InputAction* InputComponent::TryGetAction(const std::string& name) const
	{
		auto it = m_actions.find(name);
		if (it != m_actions.end()) return &(it->second);

		return nullptr;
	}
	const std::unordered_map<std::string, Input::InputAction>& InputComponent::GetActions() const
	{
		return m_actions;
	}

	void InputComponent::InitFields()
	{
		//TODO: implement
		m_Fields = {};
	}

	void InputComponent::Serialize(Serialization::Serializer& serializer) const
	{
		//TODO: implement
	}
	void InputComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
		//TODO: implement
	}
}
