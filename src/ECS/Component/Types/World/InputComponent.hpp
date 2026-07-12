#pragma once
#include <string>
#include <unordered_map>
#include "ECS/Component/Component.hpp"
#include "Core/Input/InputAction.hpp"
#include "Core/Input/InputManager.hpp"

namespace Engine::Input
{
	class InputComponent : public ECS::Component
	{
	private:
		//Input::InputManager* m_inputManager;
		std::unordered_map<std::string, Input::InputAction> m_actions;

	public:
		InputComponent();

		void AddAction(const Input::InputAction& action);
		const Input::InputAction* TryGetAction(const std::string& name) const;
		const std::unordered_map<std::string, Input::InputAction>& GetActions() const;

		//std::vector<std::string> GetDependencyFlags() const override;
		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
	};
}
