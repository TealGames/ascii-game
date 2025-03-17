#include "pch.hpp"
#include "SerializableField.hpp"

SerializableField::SerializableField() : SerializableField("", "", -1, "") {}
SerializableField::SerializableField(const std::string& sceneName, const std::string& entityName,
	const std::uint8_t& componentIndex, const std::string& fieldName) 
	: m_SerializedEntity(sceneName, entityName), m_ComponentIndex(componentIndex), m_FieldName(fieldName) {}
