#include "pch.hpp"
#include "Core/Serialization/SerializableComponent.hpp"
#include "Core/Analyzation/Debug.hpp"

const char* SerializableComponent::SELF_COMPONENT_ENTITY_KEYWORD = "Self";

SerializableComponent::SerializableComponent() : SerializableComponent("", "", "") {}

SerializableComponent::SerializableComponent(const SerializableEntity& serializableEntity, const std::string& compName) 
	: m_SerializedEntity(serializableEntity), m_ComponentName(compName) 
{
	if (!Assert(m_SerializedEntity.m_EntityName!= SELF_COMPONENT_ENTITY_KEYWORD, std::format("Tried to create a serializable "
		"component with args:[SerializableEntity]:{} compName:{} with prohibited entity name:{}. Use the componentname only "
		"constructor instead for self components", m_SerializedEntity.ToString(), m_ComponentName, m_SerializedEntity.m_EntityName)))
		return;
}

SerializableComponent::SerializableComponent(const std::string& sceneName, const std::string& entityName,
	const std::string& componentName) : m_SerializedEntity(sceneName, entityName), m_ComponentName(componentName) 
{
	if (!Assert(m_SerializedEntity.m_EntityName != SELF_COMPONENT_ENTITY_KEYWORD, std::format("Tried to create a serializable "
		"component with args: scene name:{} entity name:{} compName:{} with prohibited entity name:{}. Use the componentname only "
		"constructor instead for self components", sceneName, entityName, m_ComponentName, m_SerializedEntity.m_EntityName)))
		return;
}

SerializableComponent::SerializableComponent(const std::string& compName)
	: m_SerializedEntity("", SELF_COMPONENT_ENTITY_KEYWORD), m_ComponentName(compName)
{

}

std::string SerializableComponent::ToString() const
{
	return std::format("[SerializableComponent Scene:{} Entity:{} Comp:{}]", 
		m_SerializedEntity.m_SceneName, m_SerializedEntity.m_EntityName, m_ComponentName);
}

bool SerializableComponent::IsComponentOfEntitySelf() const
{
	return m_SerializedEntity.m_EntityName == SELF_COMPONENT_ENTITY_KEYWORD 
		&& m_SerializedEntity.m_SceneName == "";
}