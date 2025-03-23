#pragma once
#include "SerializableComponent.hpp"
#include <cstdint>

struct SerializableField
{
	SerializableComponent m_SerializedComponent;
	std::string m_FieldName;

	SerializableField();
	SerializableField(const std::string& sceneName, const std::string& entityName,
		const std::string& componentName, const std::string& fieldName);
};
