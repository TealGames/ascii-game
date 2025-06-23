#include "pch.hpp"
#include "Component.hpp"
#include "Debug.hpp"
#include "EntityData.hpp"

Component::Component() 
	: m_MutatedThisFrame(false), m_IsEnabled(true), m_entity(nullptr), m_Fields() //m_dependencyLevel(dependency)
{
}

EntityData& Component::GetEntityMutable()
{
	if (!Assert(this, m_entity != nullptr, std::format("Tried to retrieve entity from component safely but it is NULLPTR "
		"(it means a function creating or adding component probably did not update this setting)")))
		throw std::invalid_argument("Tried to retrieve invalid entity with component");

	return *m_entity;
}

const EntityData& Component::GetEntity() const
{
	if (!Assert(this, m_entity != nullptr, std::format("Tried to retrieve entity from component safely but it is NULLPTR "
		"(it means a function creating or adding component probably did not update this setting)")))
		throw std::invalid_argument("Tried to retrieve invalid entity with component");

	return *m_entity;
}

TransformData& Component::GetTransformMutable() { return GetEntityMutable().GetTransformMutable(); }
const TransformData& Component::GetTransform() const { return GetEntity().GetTransform(); }
ECS::EntityID Component::GetEntityID() const { return GetEntity().GetId(); }

void Component::InitFields()
{
	return;
}

std::vector<ComponentField>& Component::GetFieldsMutable()
{
	return m_Fields;
}

const std::vector<ComponentField>& Component::GetFields() const
{
	return m_Fields;
}

ComponentField* Component::TryGetFieldMutable(const std::string& name)
{
	for (auto& field : m_Fields)
	{
		if (field.m_FieldName == name) 
			return &field;
	}
	return nullptr;
}
const ComponentField* Component::TryGetField(const std::string& name) const
{
	for (const auto& field : m_Fields)
	{
		if (field.m_FieldName == name)
			return &field;
	}
	return nullptr;
}

std::string Component::ToStringFields() const
{
	std::vector<std::string> fieldStrings = {};
	for (const auto& field : m_Fields)
	{
		fieldStrings.emplace_back(field.ToString());
	}
	return Utils::ToStringIterable<std::vector<std::string>, std::string>(fieldStrings);
}

bool Component::Validate()
{
	return true;
}

std::string Component::ToString() const
{
	return std::format("[BaseComp Entity:{}]", GetEntity().m_Name);
}