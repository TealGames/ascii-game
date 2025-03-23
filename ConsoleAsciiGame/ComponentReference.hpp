#pragma once
#include "Entity.hpp"

struct ComponentReference
{
	ECS::Entity* m_Entity;
	//TODO: should component data be stored here?
	size_t m_ComponentIndex;

	ComponentReference();
	ComponentReference(ComponentData* componentData);
	ComponentReference(ECS::Entity& entity, const size_t& componentIndex);
	ComponentReference(ECS::Entity& entity, const std::string& componentName);

	const ECS::Entity& GetEntitySafe() const;
	ComponentData* GetComponentDataMutable();
	std::string GetComponentName() const;

	std::string ToString() const;
};

