#include "pch.hpp"
#include "Unused/EntityMapper.hpp"
#include "ECS/Component/ComponentType.hpp"
#include "Utils/HelperFunctions.hpp"


namespace Old
{
	EntityMapper::EntityMapper() :
		m_allComponents{}, m_nextAvailableComponentIndices{},
		m_entityComponentIds{}, m_nextAvailableEntityID(0)
	{
		m_entityComponentIds.reserve(MAX_ENTITIES);
	}

	ComponentRowIndex EntityMapper::GetRowIndexForComponentID(const ComponentID& compId) const
	{
		return compId / MAX_ENTITIES;
	}

	ComponentIndex EntityMapper::GetColIndexForComponentID(const ComponentID& compId) const
	{
		return compId % MAX_ENTITIES;
	}

	ComponentType EntityMapper::GetComponentTypeFromID(const ComponentID& compId) const
	{
		return GetComponentTypeByPlace(GetRowIndexForComponentID(compId));
	}

	ComponentRowIndex EntityMapper::GetRowIndexForComponentType(const ComponentType& type) const
	{
		return GetPlaceOfComponentType(type);
	}

	ComponentID EntityMapper::CreateComponentID(const ComponentType& type, const ComponentIndex& colIndex) const
	{
		return GetRowIndexForComponentType(type) * MAX_ENTITIES + colIndex;
	}

	EntityComponentsCollection::iterator EntityMapper::GetEntityIteratorMutable(const EntityID& entityId)
	{
		return m_entityComponentIds.find(entityId);
	}

	EntityComponentsCollection::const_iterator EntityMapper::GetEntityIterator(const EntityID& entityId) const
	{
		return m_entityComponentIds.find(entityId);
	}

	bool EntityMapper::HasEntityWithID(const EntityID& entityID) const
	{
		return GetEntityIterator(entityID) != m_entityComponentIds.end();
	}

	void EntityMapper::FindNextAvailableEntityID()
	{
		while (m_entityComponentIds.find(m_nextAvailableEntityID) != m_entityComponentIds.end()
			&& m_nextAvailableEntityID < MAX_ENTITY_ID)
		{
			m_nextAvailableEntityID++;
		}
	}

	EntityID EntityMapper::ReserveAvailableEntityID()
	{
		if (!Assert(this, m_nextAvailableEntityID < MAX_ENTITIES, std::format("ENTITY MAPPER: Tried to reserve the next available "
			"entity ID (current: {}), but max entities: {} have been reserved",
			std::to_string(m_nextAvailableEntityID), std::to_string(MAX_ENTITIES))))
		{
			return -1;
		}

		const EntityID currentReserved = m_nextAvailableEntityID;
		m_entityComponentIds.emplace(currentReserved, std::unordered_map<ComponentType, ComponentID>{});
		FindNextAvailableEntityID();

		return currentReserved;
	}

	void EntityMapper::FindNextAvailableComponentIndex(const ComponentRowIndex& rowIndex)
	{
		ComponentIndex newIndex = m_nextAvailableComponentIndices[rowIndex];
		while (m_componentEntityIds.find(newIndex) != m_componentEntityIds.end()
			&& newIndex < MAX_COMPONENT_COL_INDEX)
		{
			m_nextAvailableComponentIndices[rowIndex]++;
			newIndex = m_nextAvailableComponentIndices[rowIndex];
		}
	}

	void EntityMapper::StoreComponentForEntity(const EntityID& entityId, const ComponentID& compId)
	{
		auto iterator = GetEntityIteratorMutable(entityId);
		const ComponentType compType = GetComponentTypeFromID(compId);
		//If it is not found, it means this is the first component for this entity
		//so we add that, otherwise we 
		if (iterator == m_entityComponentIds.end())
		{
			m_entityComponentIds.emplace(entityId, std::unordered_map<ComponentType, ComponentID>{ {compType, compId} });
			if (entityId == m_nextAvailableEntityID) FindNextAvailableEntityID();
		}
		else
		{
			iterator->second.emplace(compType, compId);
		}

		m_componentEntityIds.emplace(compId, entityId);
	}

	std::optional<EntityID> EntityMapper::TryGetEntityWithComponent(const ComponentID& compId)
	{
		auto iterator = m_componentEntityIds.find(compId);
		if (iterator == m_componentEntityIds.end()) return std::nullopt;
		return iterator->second;
	}

	std::string EntityMapper::ToStringData(bool doEntityComponents) const
	{
		std::string result = "";
		if (doEntityComponents)
		{
			for (const auto& entityComp : m_entityComponentIds)
			{
				result += std::format("[{}, {}]",
					std::to_string(entityComp.first), Utils::ToStringIterable(entityComp.second));
			}
		}
		else
		{
			for (const auto& entityComp : m_componentEntityIds)
			{
				result += std::format("[{}{}, {}]",
					std::to_string(entityComp.first), ToString(GetComponentTypeFromID(entityComp.first)), std::to_string(entityComp.second));
			}
		}
		return result;
	}
}

