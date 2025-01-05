#include "pch.hpp"
#include "EntityMapper.hpp"
#include "Entity.hpp"
#include "Scene.hpp"
#include "GlobalEntityManager.hpp"

GlobalEntityManager::GlobalEntityManager() :
	m_globalEntities{}, m_globalEntitiesLookup{}, m_globalEntityMapper()
{}

int GlobalEntityManager::GetCount() const
{
	return m_globalEntities.size();
}

EntityCollection::iterator GlobalEntityManager::GetGlobalEntityIteratorMutable(const EntityID& id)
{
	return m_globalEntitiesLookup.find(id);
}

bool GlobalEntityManager::IsValidIterator(const EntityCollection::iterator& iterator)
{
	return iterator != m_globalEntitiesLookup.end();
}

EntityCollection::const_iterator GlobalEntityManager::GetGlobalEntityIterator(const EntityID& id) const
{
	return m_globalEntitiesLookup.find(id);
}

bool GlobalEntityManager::IsValidIterator(const EntityCollection::const_iterator& iterator) const
{
	return iterator != m_globalEntitiesLookup.end();
}

bool GlobalEntityManager::HasGlobalEntity(const EntityID& id) const
{
	return GetGlobalEntityIterator(id) != m_globalEntitiesLookup.end();
}

ECS::Entity& GlobalEntityManager::CreateGlobalEntity(const std::string& name, const TransformData& transform)
{
	m_globalEntities.emplace_back(name, m_globalEntityMapper, transform);
	m_globalEntitiesLookup.emplace(m_globalEntities.back().m_Id, &(m_globalEntities.back()));
	return m_globalEntities.back();
}

ECS::Entity& GlobalEntityManager::CreateGlobalEntity(const std::string& name, TransformData&& transform)
{
	m_globalEntities.emplace_back(name, m_globalEntityMapper, transform);
	m_globalEntitiesLookup.emplace(m_globalEntities.back().m_Id, &(m_globalEntities.back()));
	return m_globalEntities.back();
}

ECS::Entity* GlobalEntityManager::TryGetGlobalEntityMutable(const EntityID& id)
{
	auto iterator = GetGlobalEntityIteratorMutable(id);
	if (iterator == m_globalEntitiesLookup.end()) return nullptr;
	return iterator->second;
}

const ECS::Entity* GlobalEntityManager::TryGetGlobalEntity(const EntityID& id) const
{
	auto iterator = GetGlobalEntityIterator(id);
	if (iterator == m_globalEntitiesLookup.end()) return nullptr;
	return iterator->second;
}

