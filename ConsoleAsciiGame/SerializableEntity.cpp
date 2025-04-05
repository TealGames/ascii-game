#include "pch.hpp"
#include "SerializableEntity.hpp"

SerializableEntity::SerializableEntity() : SerializableEntity("", "") {}
SerializableEntity::SerializableEntity(const std::string& sceneName, const std::string& entityName) 
	: m_SceneName(sceneName), m_EntityName(entityName) {}

std::string SerializableEntity::ToString() const
{
	return std::format("[SerializableEntity Scene:{} Entity:{}]", m_SceneName, m_EntityName);
}