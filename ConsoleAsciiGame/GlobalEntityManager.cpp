#include "pch.hpp"
#include "EntityMapper.hpp"
#include "Entity.hpp"
#include "Scene.hpp"
#include "GlobalEntityManager.hpp"
#include "StringUtil.hpp"
#include "HelperFunctions.hpp"

GlobalEntityManager::GlobalEntityManager() :
	m_globalEntities{}, m_globalEntityIds{}, m_globalEntityMapper()
{
	m_globalEntities.reserve(MAX_ENTITIES);
	m_globalEntityIds.reserve(MAX_ENTITIES);
}

int GlobalEntityManager::GetCount() const
{
	return m_globalEntities.size();
}

std::string GlobalEntityManager::ToStringEntityData() const
{
	return "";
	//return m_globalEntityMapper.ToStringData();
}

std::string GlobalEntityManager::CleanName(const std::string name) const
{
	Utils::StringUtil stringUtils(name);
	return stringUtils.ToLowerCase().Trim().RemoveChar(' ').ToString();
}

EntityIDCollection::iterator GlobalEntityManager::GetGlobalEntityIteratorMutable(const ECS::EntityID& id)
{
	return m_globalEntityIds.find(id);
}
EntityIDCollection::const_iterator GlobalEntityManager::GetGlobalEntityIterator(const ECS::EntityID& id) const
{
	return m_globalEntityIds.find(id);
}
EntityNameCollection::iterator GlobalEntityManager::GetGlobalEntityIteratorMutable(const std::string& name)
{
	std::string cleanedName = CleanName(name);
	return m_globalEntityNames.find(cleanedName);
}
EntityNameCollection::const_iterator GlobalEntityManager::GetGlobalEntityIterator(const std::string& name) const
{
	std::string cleanedName = CleanName(name);
	return m_globalEntityNames.find(cleanedName);
}

bool GlobalEntityManager::IsValidIterator(const EntityIDCollection::iterator& iterator)
{
	return iterator != m_globalEntityIds.end();
}
bool GlobalEntityManager::IsValidIterator(const EntityIDCollection::const_iterator& iterator) const
{
	return iterator != m_globalEntityIds.end();
}
bool GlobalEntityManager::IsValidIterator(const EntityNameCollection::iterator& iterator)
{
	return iterator != m_globalEntityNames.end();
}
bool GlobalEntityManager::IsValidIterator(const EntityNameCollection::const_iterator& iterator) const
{
	return iterator != m_globalEntityNames.end();
}

bool GlobalEntityManager::HasGlobalEntity(const ECS::EntityID& id) const
{
	return IsValidIterator(GetGlobalEntityIterator(id));
}
bool GlobalEntityManager::HasGlobalEntity(const std::string& name, const bool& cleanName) const
{
	return IsValidIterator(GetGlobalEntityIterator(cleanName? CleanName(name) : name));
}
bool GlobalEntityManager::HasGlobalEntity(const std::string& name) const
{
	return HasGlobalEntity(name, true);
}

ECS::Entity& GlobalEntityManager::CreateGlobalEntity(const std::string& name, const TransformData& transform)
{
	std::string cleanedName = CleanName(name);
	//Since we want cleaned name to use in error message, we choose to not clean second time when checking for entity
	Utils::Assert(this, !HasGlobalEntity(name, false), 
		std::format("Tried to create a global entity with name: {} (cleaned:{}) that conflicts with existing global entity. "
		"Note: it will still be added but will ruin the use of entity name searching!", name, cleanedName));

	m_globalEntities.emplace_back(name, m_globalEntityMapper, transform);
	m_globalEntityIds.emplace(m_globalEntities.back().m_Id, &(m_globalEntities.back()));
	m_globalEntityNames.emplace(cleanedName, &(m_globalEntities.back()));
	m_globalEntities.back().m_Transform.m_Entity = &(m_globalEntities.back());

	return m_globalEntities.back();
}

ECS::Entity& GlobalEntityManager::CreateGlobalEntity(const std::string& name, TransformData&& transform)
{
	std::string cleanedName = CleanName(name);
	//Since we want cleaned name to use in error message, we choose to not clean second time when checking for entity
	Utils::Assert(this, !HasGlobalEntity(cleanedName, false), 
		std::format("Tried to create a global entity with name: {} (cleaned:{}) that conflicts with existing global entity. "
		"Note: it will still be added but will ruin the use of entity name searching!", name, cleanedName));

	m_globalEntities.emplace_back(name, m_globalEntityMapper, std::move(transform));
	m_globalEntityIds.emplace(m_globalEntities.back().m_Id, &(m_globalEntities.back()));
	m_globalEntityNames.emplace(cleanedName, &(m_globalEntities.back()));
	m_globalEntities.back().m_Transform.m_Entity = &(m_globalEntities.back());

	return m_globalEntities.back();
}

ECS::Entity* GlobalEntityManager::TryGetGlobalEntityMutable(const ECS::EntityID& id)
{
	auto iterator = GetGlobalEntityIteratorMutable(id);
	if (IsValidIterator(iterator)) return iterator->second;
	return nullptr;
}
ECS::Entity* GlobalEntityManager::TryGetGlobalEntityMutable(const std::string& name)
{
	auto iterator = GetGlobalEntityIteratorMutable(name);
	if (IsValidIterator(iterator)) iterator->second; 
	return nullptr;
}

const ECS::Entity* GlobalEntityManager::TryGetGlobalEntity(const ECS::EntityID& id) const
{
	auto iterator = GetGlobalEntityIterator(id);
	if (IsValidIterator(iterator)) return iterator->second;
	return nullptr;
}
const ECS::Entity* GlobalEntityManager::TryGetGlobalEntity(const std::string& name) const
{
	auto iterator = GetGlobalEntityIterator(name);
	if (IsValidIterator(iterator)) return iterator->second;
	return nullptr;
}

const std::vector<ECS::Entity>& GlobalEntityManager::GetAllGlobalEntities() const
{
	return m_globalEntities;
}

std::vector<ECS::Entity>& GlobalEntityManager::GetAllGlobalEntitiesMutable()
{
	return m_globalEntities;
}

