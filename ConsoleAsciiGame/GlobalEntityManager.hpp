#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "MultiBodySystem.hpp"
#include <optional>
#include <functional>
#include <format>
#include <entt/entt.hpp>
//#include "EntityMapper.hpp"
#include "Entity.hpp"
#include "Scene.hpp"
#include "HelperFunctions.hpp"
//#include "AssetManager.hpp"

using EntityIDCollection = std::unordered_map<ECS::EntityID, ECS::Entity*>;
using EntityNameCollection = std::unordered_map<std::string, ECS::Entity*>;
class GlobalEntityManager
{
private:
	//AssetManager& m_assetManager;

	//EntityMapper m_globalEntityMapper;
	entt::registry m_globalEntityMapper;
	std::vector<ECS::Entity> m_globalEntities;
	EntityIDCollection m_globalEntityIds;
	EntityNameCollection m_globalEntityNames;

public:
	static const char* GLOBAL_SCENE_NAME;

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

	EntityIDCollection::iterator GetGlobalEntityIteratorMutable(const ECS::EntityID& id);
	EntityNameCollection::iterator GetGlobalEntityIteratorMutable(const std::string& name);
	bool IsValidIterator(const EntityIDCollection::iterator& iterator);
	bool IsValidIterator(const EntityNameCollection::iterator& iterator);

	EntityIDCollection::const_iterator GetGlobalEntityIterator(const ECS::EntityID& id) const;
	EntityNameCollection::const_iterator GetGlobalEntityIterator(const std::string& name) const;
	bool IsValidIterator(const EntityIDCollection::const_iterator& iterator) const;
	bool IsValidIterator(const EntityNameCollection::const_iterator& iterator) const;

	bool HasGlobalEntity(const ECS::EntityID& id) const;
	bool HasGlobalEntity(const std::string& name) const;
	
	ECS::Entity& CreateGlobalEntity(const std::string& name, const TransformData& transform);
	ECS::Entity& CreateGlobalEntity(const std::string& name, TransformData&& transform);

	ECS::Entity* TryGetGlobalEntityMutable(const ECS::EntityID& id);
	ECS::Entity* TryGetGlobalEntityMutable(const std::string& name);
	const ECS::Entity* TryGetGlobalEntity(const ECS::EntityID& id) const;
	const ECS::Entity* TryGetGlobalEntity(const std::string& name) const;

	const std::vector<ECS::Entity>& GetAllGlobalEntities() const;
	std::vector<ECS::Entity>& GetAllGlobalEntitiesMutable();

	/*template<typename T>
	std::vector<T*> TryGetComponentsOfType(const ComponentType& type, 
		const std::function<void(T*, const EntityID&)>& action = nullptr)
	{
		return m_globalEntityMapper.TryGetComponentsOfType<T>(type, action);
	}*/

	/*template<typename T>
	bool AddComponent(ECS::Entity& entity, const T& component)
	{
		return entity.AddComponent<T>(component);
	}

	template<typename T>
	bool AddComponent(const ECS::EntityID& id, const T& component)
	{
		auto iterator = GetGlobalEntityIteratorMutable(id);
		if (!Assert(this, IsValidIterator(iterator), std::format("Tried to add component: {} "
			"to global entity by ID: {} but it does not exist as a global entity",
			typeid(T).name(), std::to_string(id)))) return false;

		return iterator->second->AddComponent<T>(component);
	}

	template<typename T>
	bool AddComponent(const std::string& name, const T& component)
	{
		auto iterator = GetGlobalEntityIteratorMutable(name);
		if (!Assert(this, IsValidIterator(iterator), std::format("Tried to add component: {} "
			"to global entity by NAME: {} but it does not exist as a global entity",
			typeid(T).name(), name))) return false;

		return iterator->second->AddComponent<T>(component);
	}*/

	template<typename T>
	requires ECS::IsComponent<T>
	void OperateOnComponents(const std::function<void(T&, ECS::Entity&)> action)
	{
		ECS::OperateOnActiveComponents<T>(m_globalEntityMapper,
			[this](const ECS::EntityID id)->ECS::Entity*
			{
				return TryGetGlobalEntityMutable(id);
			}, action);

		/*
		auto view = m_globalEntityMapper.view<T>();
		for (auto entityId : view)
		{
			ECS::Entity* entityPtr = TryGetGlobalEntityMutable(entityId);
			if (!Assert(this, entityPtr != nullptr, std::format("Tried to operate on component type: {} "
				"but failed to retrieve entity with ID: {} (it probably does not exist in the scene)",
				typeid(T).name(), ECS::Entity::ToString(entityId)))) 
				return;

			if (!entityPtr->m_Active) continue;

			T* component = &(view.get<T>(entityId));
			ComponentData* componentBase = static_cast<ComponentData*>(component);
			if (!componentBase->m_IsEnabled) continue;

			action(*component, *entityPtr);
		}
		*/
		/*const ComponentType type = GetComponentFromType<T>();
		if (!Assert(type != ComponentType::None, std::format("Tried to operate on component type: {} "
			"but failed to retrieve its enum value", typeid(T).name()))) return;

		std::function<void(T*, const EntityID&)> actions = [this, &action](T* component, const EntityID& id)-> void
			{
				ECS::Entity* entityPtr = TryGetEntity(id);
				if (!Assert(entityPtr != nullptr, std::format("Tried to operate on component type: {} "
					"but failed to retrieve entity with ID: {} (it probably does not exist in the scene)",
					typeid(T).name(), std::to_string(id)))) return;

				action(*component, *entityPtr);
			};

		m_entityMapper.TryGetComponentsOfType<T>(type,actions);
		m_globalEntities.TryGetComponentsOfType<T>(type,actions);*/
	}
};

