#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include <functional>
#include <format>
#include "EntityMapper.hpp"
#include "Entity.hpp"
#include "Scene.hpp"
#include "HelperFunctions.hpp"

using EntityIDCollection = std::unordered_map<EntityID, ECS::Entity*>;
using EntityNameCollection = std::unordered_map<std::string, ECS::Entity*>;
class GlobalEntityManager
{
private:
	EntityMapper m_globalEntityMapper;
	std::vector<ECS::Entity> m_globalEntities;
	EntityIDCollection m_globalEntityIds;
	EntityNameCollection m_globalEntityNames;

public:

private:
	/// <summary>
	/// Will remove spaces, convert to lowercase
	/// </summary>
	/// <param name="name"></param>
	/// <returns></returns>
	std::string CleanName(const std::string name) const;
	bool HasGlobalEntity(const std::string& name, const bool& cleanName) const;

public:
	GlobalEntityManager();

	int GetCount() const;
	std::string ToStringEntityData() const;

	EntityIDCollection::iterator GetGlobalEntityIteratorMutable(const EntityID& id);
	EntityNameCollection::iterator GetGlobalEntityIteratorMutable(const std::string& name);
	bool IsValidIterator(const EntityIDCollection::iterator& iterator);
	bool IsValidIterator(const EntityNameCollection::iterator& iterator);

	EntityIDCollection::const_iterator GetGlobalEntityIterator(const EntityID& id) const;
	EntityNameCollection::const_iterator GetGlobalEntityIterator(const std::string& name) const;
	bool IsValidIterator(const EntityIDCollection::const_iterator& iterator) const;
	bool IsValidIterator(const EntityNameCollection::const_iterator& iterator) const;


	bool HasGlobalEntity(const EntityID& id) const;
	bool HasGlobalEntity(const std::string& name) const;
	
	ECS::Entity& CreateGlobalEntity(const std::string& name, const TransformData& transform);
	ECS::Entity& CreateGlobalEntity(const std::string& name, TransformData&& transform);

	ECS::Entity* TryGetGlobalEntityMutable(const EntityID& id);
	ECS::Entity* TryGetGlobalEntityMutable(const std::string& name);
	const ECS::Entity* TryGetGlobalEntity(const EntityID& id) const;
	const ECS::Entity* TryGetGlobalEntity(const std::string& name) const;

	template<typename T>
	std::vector<T*> TryGetComponentsOfType(const ComponentType& type, 
		const std::function<void(T*, const EntityID&)>& action = nullptr)
	{
		return m_globalEntityMapper.TryGetComponentsOfType<T>(type, action);
	}

	template<typename T>
	bool TryAddComponent(ECS::Entity& entity, T& component)
	{
		return entity.TryAddComponent<T>(component);
	}

	template<typename T>
	bool TryAddComponent(const EntityID& id, T& component)
	{
		auto iterator = GetGlobalEntityIteratorMutable(id);
		if (!Utils::Assert(this, IsValidIterator(iterator), std::format("Tried to add component: {} "
			"to global entity by ID: {} but it does not exist as a global entity",
			typeid(T).name(), std::to_string(id)))) return false;

		return iterator->second->TryAddComponent<T>();
	}

	template<typename T>
	bool TryAddComponent(const std::string& name, T& component)
	{
		auto iterator = GetGlobalEntityIteratorMutable(name);
		if (!Utils::Assert(this, IsValidIterator(iterator), std::format("Tried to add component: {} "
			"to global entity by NAME: {} but it does not exist as a global entity",
			typeid(T).name(), name))) return false;

		return iterator->second->TryAddComponent<T>();
	}
};

