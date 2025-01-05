#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include <format>
#include "EntityMapper.hpp"
#include "Entity.hpp"
#include "Scene.hpp"
#include "HelperFunctions.hpp"

using EntityCollection = std::unordered_map<EntityID, ECS::Entity*>;
class GlobalEntityManager
{
private:
	EntityMapper m_globalEntityMapper;
	std::vector<ECS::Entity> m_globalEntities;
	EntityCollection m_globalEntitiesLookup;

public:

private:

public:
	GlobalEntityManager();

	int GetCount() const;

	EntityCollection::iterator GetGlobalEntityIteratorMutable(const EntityID& id);
	bool IsValidIterator(const EntityCollection::iterator& iterator);

	EntityCollection::const_iterator GetGlobalEntityIterator(const EntityID& id) const;
	bool IsValidIterator(const EntityCollection::const_iterator& iterator) const;

	bool HasGlobalEntity(const EntityID& id) const;
	ECS::Entity& CreateGlobalEntity(const std::string& name, const TransformData& transform);
	ECS::Entity& CreateGlobalEntity(const std::string& name, TransformData&& transform);

	ECS::Entity* TryGetGlobalEntityMutable(const EntityID& id);
	const ECS::Entity* TryGetGlobalEntity(const EntityID& id) const;

	template<typename T>
	bool TryAddComponent(ECS::Entity& entity, T& component)
	{
		return entity.TryAddComponent<T>(component);
	}

	template<typename T>
	bool TryAddComponent(EntityID& id, T& component)
	{
		auto iterator = GetGlobalEntityIteratorMutable(id);
		if (!Utils::Assert(iterator != m_globalEntitiesLookup.end(), std::format("Tried to add component: {} "
			"to global entity with id: {} but it does not exist as a global entity",
			typeid(T).name(), std::to_string(id)))) return false;

		return iterator->second->TryAddComponent<T>();
	}
};

