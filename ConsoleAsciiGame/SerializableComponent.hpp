#pragma once
#include "SerializableEntity.hpp"
#include <cstdint>

struct SerializableComponent
{
	SerializableEntity m_SerializedEntity;
	std::string m_ComponentName;

	SerializableComponent();
	SerializableComponent(const std::string& sceneName, const std::string& entityName,
		const std::string& componentName);
};


