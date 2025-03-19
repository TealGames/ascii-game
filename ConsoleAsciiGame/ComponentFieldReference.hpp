#pragma once
#include "Entity.hpp"

struct ComponentFieldReference
{
	ECS::Entity* m_Entity;
	//TODO: should component data be stored here?
	size_t m_ComponentIndex;
	ComponentField* m_FieldRef;

	ComponentFieldReference();
	ComponentFieldReference(ComponentData* componentData, const std::string& fieldName);
	ComponentFieldReference(ECS::Entity& entity, const size_t& componentIndex, const std::string& fieldName);

	const ECS::Entity& GetEntitySafe() const;
	const ComponentField& GetComponentFieldSafe() const;

	ComponentField& GetComponentFieldSafeMutable();
	ComponentData* GetComponentDataMutable();

	std::string GetFieldName() const;
};

