#pragma once
#include "EntityData.hpp"
#include "ComponentReference.hpp"

struct ComponentFieldReference
{
	//ECS::Entity* m_Entity;
	ComponentReference m_ComponentRef;
	//TODO: should component data be stored here?
	//size_t m_ComponentIndex;
	ComponentField* m_FieldRef;

	ComponentFieldReference();
	ComponentFieldReference(ComponentData* componentData, const std::string& fieldName);
	ComponentFieldReference(EntityData& entity, const std::string& componentName, const std::string& fieldName);

	const EntityData& GetEntitySafe() const;
	const ComponentField& GetComponentFieldSafe() const;

	ComponentField& GetComponentFieldSafeMutable();
	ComponentData* GetComponentDataMutable();

	std::string GetFieldName() const;
};

