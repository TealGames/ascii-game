#include "pch.hpp"
#include "ComponentData.hpp"
#include "Debug.hpp"
#include "EntityData.hpp"

ComponentData::ComponentData() 
	: m_MutatedThisFrame(false), m_IsEnabled(true), m_entity(nullptr), m_Fields() //m_dependencyLevel(dependency)
{
}

//HighestDependecyLevel ComponentData::GetDependencyLevel() const
//{
//	return m_dependencyLevel;
//}

//bool ComponentData::DependsOnEntity() const
//{
//	auto dependencies = GetDependencyFlags();
//	return !dependencies.empty() && dependencies[0] == ENTITY_DEPENDENCY_FLAG;
//}
//bool ComponentData::DependsOnAnySiblingComponent() const
//{
//	auto dependencies = GetDependencyFlags();
//	return !dependencies.empty() && dependencies[0] != ENTITY_DEPENDENCY_FLAG;
//}
//bool ComponentData::HasDependencies() const
//{
//	auto dependencies = GetDependencyFlags();
//	return !dependencies.empty();
//}
//std::vector<std::string> ComponentData::GetComponentDependencies() const
//{
//	if (DependsOnAnySiblingComponent()) return GetDependencyFlags();
//	return {};
//}
//bool ComponentData::DoesEntityHaveComponentDependencies() const
//{
//	if (!DependsOnAnySiblingComponent()) return false;
//
//	const EntityData& thisEntity = GetEntitySafe();
//	for (const auto& compDependency : GetDependencyFlags())
//	{
//		if (!thisEntity.HasComponent(compDependency))
//			return false;
//	}
//	return true;
//}

EntityData& ComponentData::GetEntitySafeMutable()
{
	if (!Assert(this, m_entity != nullptr, std::format("Tried to retrieve entity from component safely but it is NULLPTR "
		"(it means a function creating or adding component probably did not update this setting)")))
		throw std::invalid_argument("Tried to retrieve invalid entity with component");

	return *m_entity;
}

const EntityData& ComponentData::GetEntitySafe() const
{
	if (!Assert(this, m_entity != nullptr, std::format("Tried to retrieve entity from component safely but it is NULLPTR "
		"(it means a function creating or adding component probably did not update this setting)")))
		throw std::invalid_argument("Tried to retrieve invalid entity with component");

	return *m_entity;
}

TransformData& ComponentData::GetTransformMutable() { return GetEntitySafeMutable().GetTransformMutable(); }
const TransformData& ComponentData::GetTransform() const { return GetEntitySafe().GetTransform(); }
ECS::EntityID ComponentData::GetEntityID() const { return GetEntitySafe().GetId(); }

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
		fieldStrings.emplace_back(field.ToString());
	}
	return Utils::ToStringIterable<std::vector<std::string>, std::string>(fieldStrings);
}

bool ComponentData::Validate()
{
	return true;
}

std::string ComponentData::ToString() const
{
	return std::format("[BaseComp Entity:{}]", GetEntitySafe().m_Name);
}