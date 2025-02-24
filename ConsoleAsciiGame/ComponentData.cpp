#include "pch.hpp"
#include "ComponentData.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"

ComponentData::ComponentData() : m_MutatedThisFrame(false), m_Entity(nullptr), m_Fields() {}

ECS::Entity& ComponentData::GetEntitySafeMutable()
{
	if (!Assert(this, m_Entity != nullptr, std::format("Tried to retrieve entity from component safely but it is NULLPTR "
		"(it means a function creating or adding component probably did not update this setting)")))
		throw std::invalid_argument("Tried to retrieve invalid entity with component");

	return *m_Entity;
}

const ECS::Entity& ComponentData::GetEntitySafe() const
{
	if (!Assert(this, m_Entity != nullptr, std::format("Tried to retrieve entity from component safely but it is NULLPTR "
		"(it means a function creating or adding component probably did not update this setting)")))
		throw std::invalid_argument("Tried to retrieve invalid entity with component");

	return *m_Entity;
}

void ComponentData::InitFields()
{
	return;
}

std::vector<ComponentField>& ComponentData::GetFieldsMutable()
{
	return m_Fields;
}

const std::vector<ComponentField>& ComponentData::GetFields() const
{
	return m_Fields;
}