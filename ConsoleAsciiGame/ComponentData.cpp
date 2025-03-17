#include "pch.hpp"
#include "Entity.hpp"
#include "ComponentData.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"

ComponentData::ComponentData(const HighestDependecyLevel& dependency) 
	: m_dependencyLevel(dependency), m_MutatedThisFrame(false), m_Entity(nullptr), m_Fields() 
{
}

HighestDependecyLevel ComponentData::GetDependencyLevel() const
{
	return m_dependencyLevel;
}

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

ComponentField* ComponentData::TryGetFieldMutable(const std::string& name)
{
	for (auto& field : m_Fields)
	{
		if (field.m_FieldName == name) 
			return &field;
	}
	return nullptr;
}
const ComponentField* ComponentData::TryGetField(const std::string& name) const
{
	for (const auto& field : m_Fields)
	{
		if (field.m_FieldName == name)
			return &field;
	}
	return nullptr;
}

std::string ComponentData::ToStringFields() const
{
	std::vector<std::string> fieldStrings = {};
	for (const auto& field : m_Fields)
	{
		fieldStrings.push_back(field.ToString());
	}
	return Utils::ToStringIterable<std::vector<std::string>, std::string>(fieldStrings);
}

bool ComponentData::Validate()
{
	return true;
}

std::string ComponentData::ToString() const
{
	return std::format("[BaseComp Entity:{}]", GetEntitySafe().GetName());
}