#pragma once
#include "EntityData.hpp"

struct ComponentReference
{
	EntityData* m_Entity;
	//TODO: should component data be stored here?
	size_t m_ComponentIndex;

	ComponentReference();
	ComponentReference(Component* componentData);
	ComponentReference(EntityData& entity, const size_t& componentIndex);
	ComponentReference(EntityData& entity, const std::string& componentName);

	const EntityData& GetEntitySafe() const;
	Component* GetComponentDataMutable();
	std::string GetComponentName() const;

	std::string ToString() const;
};

