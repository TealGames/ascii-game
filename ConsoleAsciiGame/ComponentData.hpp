#pragma once
#include <vector>
#include "ComponentField.hpp"

namespace ECS
{
	class Entity;
}

struct ComponentData
{
	//TODOL: also add a static flag so we can optimize some components
	bool m_MutatedThisFrame;

	//Guaranteed to not be nullptr (but cant be a ref to allow it to be set
	//not on construction without the need to have it as constructor arg
	ECS::Entity* m_Entity;

	std::vector<ComponentField> m_Fields;

	ComponentData();
	virtual ~ComponentData() = default;
	ECS::Entity& GetEntitySafeMutable();
	const ECS::Entity& GetEntitySafe() const;

	std::vector<ComponentField>& GetFieldsMutable();

	virtual void InitFields();
	const std::vector<ComponentField>& GetFields() const;
};