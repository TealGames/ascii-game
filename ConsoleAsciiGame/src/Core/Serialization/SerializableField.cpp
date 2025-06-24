#include "pch.hpp"
#include "Core/Serialization/SerializableField.hpp"

SerializableField::SerializableField() : SerializableField("", "", "", "") {}
SerializableField::SerializableField(const SerializableComponent& serializedComp, const std::string& fieldName)
: m_SerializedComponent(serializedComp), m_FieldName(fieldName) {}

SerializableField::SerializableField(const std::string& sceneName, const std::string& entityName,
	const std::string& componentName, const std::string& fieldName) 
	: m_SerializedComponent(sceneName, entityName, componentName), m_FieldName(fieldName) {}
