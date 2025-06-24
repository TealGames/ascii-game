#include "pch.hpp"
#include "ComponentReference.hpp"
#include "Debug.hpp"

ComponentReference::ComponentReference() 
	: m_Entity(nullptr), m_ComponentIndex(-1) {}

ComponentReference::ComponentReference(Component* componentData) 
	: m_Entity(&(componentData->GetEntityMutable())), m_ComponentIndex(-1)
{
	if (!Assert(this, m_Entity != nullptr, std::format("Tried to construct a component reference with component data: {} "
		"but the entity from that component data is null", componentData->ToString())))
		return;

	m_ComponentIndex = m_Entity->TryGetIndexOfComponent(componentData);
	if (!Assert(this, m_ComponentIndex != -1, std::format("Tried to construct a field reference with entity: {} "
		"but when trying to retrieve component idnex:{} it is out of bounds!",
		m_Entity->ToString(), std::to_string(m_ComponentIndex))))
		return;
}
ComponentReference::ComponentReference(EntityData& entity, const size_t& componentIndex)
	: m_Entity(&entity), m_ComponentIndex(componentIndex)
{
	if (!Assert(this, m_Entity != nullptr, std::format("Tried to construct a component reference with entity:{} "
		"component index:{} but the entity from that component data is null", 
		entity.ToString(), std::to_string(m_ComponentIndex))))
		return;

	Component* data = m_Entity->TryGetComponentAtIndexMutable(m_ComponentIndex);
	if (!Assert(this, data != nullptr, std::format("Tried to construct a component reference with entity: {} "
		"but could not retrieve component at index:{}!", m_Entity->ToString(), std::to_string(m_ComponentIndex))))
		return;
}

ComponentReference::ComponentReference(EntityData& entity, const std::string& componentName) :
	m_Entity(&entity), m_ComponentIndex(-1)
{
	m_ComponentIndex = m_Entity->TryGetIndexOfComponent(componentName);

	if (!Assert(this, m_ComponentIndex != -1, std::format("Tried to construct a field reference with entity: {} "
		"and component name:{} but when trying to find it for entity could not be found!",
		m_Entity->ToString(), componentName)))
		return;
}

const EntityData& ComponentReference::GetEntitySafe() const
{
	if (!Assert(this, m_Entity != nullptr, std::format("Tried to get entity from component "
		"reference but it is null")))
	{
		throw std::invalid_argument("Invalid component reference entity state");
	}
		
	return *m_Entity;
}
Component* ComponentReference::GetComponentDataMutable()
{
	return m_Entity->TryGetComponentAtIndexMutable(m_ComponentIndex);
}

std::string ComponentReference::GetComponentName() const
{
	const Component* componentData = m_Entity->TryGetComponentAtIndex(m_ComponentIndex);
	return Utils::FormatTypeName(typeid(*componentData).name());
}

std::string ComponentReference::ToString() const
{
	return std::format("[ComponentRef Entity:{} CompIndex:{}]", 
		m_Entity!=nullptr? m_Entity->ToString() : "NULL", std::to_string(m_ComponentIndex));
}