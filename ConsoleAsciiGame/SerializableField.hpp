#pragma once
#include "SerializableEntity.hpp"
#include <cstdint>

struct SerializableField
{
	SerializableEntity m_SerializedEntity;
	std::uint8_t m_ComponentIndex;
	std::string m_FieldName;

	SerializableField();
	SerializableField(const std::string& sceneName, const std::string& entityName,
		const std::uint8_t& componentIndex, const std::string& fieldName);
};
