#include "pch.hpp"
#include "ComponentFieldReference.hpp"
#include "Debug.hpp"
#include "HelperFunctions.hpp"

ComponentFieldReference::ComponentFieldReference()
	: m_ComponentRef(), m_FieldRef(nullptr) {}//m_Entity(nullptr), m_ComponentRef() {}, m_ComponentIndex(-1), m_FieldRef(nullptr) {}

ComponentFieldReference::ComponentFieldReference(ComponentData* componentData, const std::string& fieldName)
	: m_ComponentRef(componentData), m_FieldRef(nullptr)//m_Entity(&(componentData->GetEntitySafeMutable())), m_ComponentRef(), m_FieldRef(nullptr), m_ComponentIndex(-1)
{
	/*if (!Assert(this, m_Entity != nullptr, std::format("Tried to construct a component field reference with component data: {} "
		"and field: {} but the entity from that component data is null", componentData->ToString(), fieldName)))
		return;

	m_ComponentIndex = m_Entity->TryGetIndexOfComponent(componentData);
	if (!Assert(this, m_ComponentIndex != -1, std::format("Tried to construct a component field reference with entity: {} "
		"and field:{} but when trying to retrieve component idnex:{} it is out of bounds!", 
		m_Entity->ToString(), fieldName, std::to_string(m_ComponentIndex))))
		return;*/

	if (!Assert(this, componentData!=nullptr, std::format("Tried to construct a component reference for field: {} "
		"with NULL component data!", fieldName))) 
		return;

	m_FieldRef = componentData->TryGetFieldMutable(fieldName);

	Assert(this, m_FieldRef != nullptr, std::format("Tried to create component field reference with component data:{} fieldNmae:{} "
		"but the resulting component data of type:{} could not find field with that name", 
		m_ComponentRef.ToString(), fieldName, typeid(*componentData).name()));
}

ComponentFieldReference::ComponentFieldReference(ECS::Entity& entity, const std::string& componentName, const std::string& fieldName)
	: m_ComponentRef(entity, componentName), m_FieldRef(nullptr) //m_Entity(&entity), m_ComponentIndex(componentIndex)
{
	/*
	if (!Assert(this, m_Entity != nullptr, std::format("Tried to construct a component field reference with entity:{} "
		"component index:{} and field: {} but the entity from that component data is null", entity.ToString(), std::to_string(m_ComponentIndex), fieldName)))
		return;
	*/

	ComponentData* data = m_ComponentRef.GetComponentDataMutable();
	if (!Assert(this, data!=nullptr, std::format("Tried to construct a component field reference with component:{} "
		"and field:{} but could not retrieve component for type:{}!", m_ComponentRef.ToString(), fieldName, componentName)))
		return;

	m_FieldRef = data->TryGetFieldMutable(fieldName);

	Assert(this, m_FieldRef != nullptr, std::format("Tried to create component field reference with component ref:{} fieldNmae:{} "
		"but the resulting component data of type:{} could not find field with that name", 
		m_ComponentRef.ToString(), fieldName, typeid(*data).name()));
}

const ECS::Entity& ComponentFieldReference::GetEntitySafe() const
{
	/*
	if (!Assert(this, m_Entity != nullptr, std::format("Tried to get entity from component "
		"field reference but it is null")))
		throw std::invalid_argument("Invalid component field reference entity state");

	return *m_Entity;
	*/
	try
	{
		return m_ComponentRef.GetEntitySafe();
	}
	catch (const std::exception& e)
	{
		LogError(this, std::format("Tried to get entity safe from component field reference "
			"but entity retrieval from component ref threw!"));
		throw std::invalid_argument("Invalid entity state");
	}
}
const ComponentField& ComponentFieldReference::GetComponentFieldSafe() const
{
	if (!Assert(this, m_FieldRef != nullptr, std::format("Tried to get component field from component "
		"field reference for entity: {} but it is NULL", GetEntitySafe().ToString())))
		throw std::invalid_argument("Invalid component field reference field state");

	return *m_FieldRef;
}
ComponentField& ComponentFieldReference::GetComponentFieldSafeMutable()
{
	if (!Assert(this, m_FieldRef != nullptr, std::format("Tried to get component field MUTABLE from component "
		"field reference for entity: {} but it is NULL", GetEntitySafe().ToString())))
		throw std::invalid_argument("Invalid component field reference field state");

	return *m_FieldRef;
}

ComponentData* ComponentFieldReference::GetComponentDataMutable()
{
	//return m_Entity->TryGetComponentAtIndexMutable(m_ComponentIndex);
	return m_ComponentRef.GetComponentDataMutable();
}

std::string ComponentFieldReference::GetFieldName() const
{
	return GetComponentFieldSafe().m_FieldName;
}