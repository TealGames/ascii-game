#include "pch.hpp"
#include "SerializableComponent.hpp"

SerializableComponent::SerializableComponent() : SerializableComponent("", "", "") {}

SerializableComponent::SerializableComponent(const std::string& sceneName, const std::string& entityName,
	const std::string& componentName) : m_SerializedEntity(sceneName, entityName), m_ComponentName(componentName) {}
