#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "ECS/Systems/MultiBodySystem.hpp"
#include <optional>
#include <functional>
#include <format>
#include "ECS/Entity/EntityRegistry.hpp"
//#include "Unused/EntityMapper.hpp"
#include "Core/Scene/Scene.hpp"
#include "Utils/HelperFunctions.hpp"
//#include "Core/Asset/AssetManager.hpp"

class EntityData;

//using EntityIDCollection = std::unordered_map<ECS::EntityID, EntityData*>;
using EntityNameCollection = std::unordered_map<std::string, EntityData*>;
class GlobalEntityManager
{
private:
	//AssetManager& m_assetManager;

	//EntityMapper m_globalEntityMapper;
	ECS::EntityRegistry m_globalRegistry;
	std::vector<EntityData*> m_globalEntities;
	//EntityIDCollection m_globalEntityIds;
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

	//EntityIDCollection::iterator GetGlobalEntityIteratorMutable(const ECS::EntityID& id);
	EntityNameCollection::iterator GetGlobalEntityIteratorMutable(const std::string& name);
	//bool IsValidIterator(const EntityIDCollection::iterator& iterator);
	bool IsValidIterator(const EntityNameCollection::iterator& iterator);

	//EntityIDCollection::const_iterator GetGlobalEntityIterator(const ECS::EntityID& id) const;
	EntityNameCollection::const_iterator GetGlobalEntityIterator(const std::string& name) const;
	//bool IsValidIterator(const EntityIDCollection::const_iterator& iterator) const;
	bool IsValidIterator(const EntityNameCollection::const_iterator& iterator) const;

	bool HasGlobalEntity(const ECS::EntityID& id) const;
	bool HasGlobalEntity(const std::string& name) const;
	
	EntityData& CreateGlobalEntity(const std::string& name, const TransformData& transform);
	//EntityData& CreateGlobalEntity(const std::string& name, TransformData&& transform);

	EntityData* TryGetGlobalEntityMutable(const ECS::EntityID& id);
	EntityData* TryGetGlobalEntityMutable(const std::string& name);
	const EntityData* TryGetGlobalEntity(const ECS::EntityID& id) const;
	const EntityData* TryGetGlobalEntity(const std::string& name) const;

	const std::vector<EntityData*>& GetAllGlobalEntities() const;
	std::vector<EntityData*>& GetAllGlobalEntitiesMutable();

	template<typename T>
	requires std::is_base_of_v<Component, T>
	void OperateOnComponents(const std::function<void(T&)> action)
	{
		ECS::OperateOnActiveComponents<T>(m_globalRegistry, action);
	}

	template<typename T>
	requires std::is_base_of_v<Component, T>
	void GetComponents(std::vector<T*>& inputVec)
	{
		ECS::GetRegistryComponentsMutable<T>(m_globalRegistry, inputVec);
	}
};

