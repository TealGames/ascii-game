#pragma once
#include <cstdint>
#include <format>
#include <vector>
#include <optional>
#include <functional>
#include <any>
#include <limits>
#include <array>
#include <set>
#include <string>
#include <algorithm>
#include <type_traits>
#include <unordered_map>
#include "HelperFunctions.hpp"
#include "ComponentType.hpp"

using EntityID = std::uint8_t;
//Although technicallty it could support max+1 due to 0, the last amount is reserved
//to indicate max limit reached for next available indices
constexpr EntityID MAX_ENTITIES = std::numeric_limits<EntityID>::max();
constexpr EntityID MAX_ENTITY_ID = MAX_ENTITIES - 1;

//Max component ID possible is max entities * number of bits for type used for representing component
//types in order to ensure each component is unique identifiable without the need for provoding component type
using ComponentID = uint16_t;

//TODO: maybe making the wrapper not rely on component types would make it better
//and instead using pure templates and creating errors if things go wrong?

//The max number of component TYPES is the same as the the max numbers reserved for component types 
// (technically numeric type's max value - 1 since last num is reserved for invalid indices)
using ComponentRowIndex = ComponentNumericType;
//The last value of the row index type is reserved for invalid indices
constexpr ComponentRowIndex MAX_COMPONENT_ROW_INDEX = MAX_COMPONENT_TYPES - 1;

using ComponentIndex = EntityID;
constexpr ComponentIndex MAX_COMPONENT_COL_INDEX = MAX_ENTITIES - 1;

//TODO: maybe using std::variant could improve performance and prevent the need for enum
using ComponentCollection = std::vector<std::any>;
//Although it is not neccessary to store the comp type with comp id since that can be retrevied from id,
//this is done to make the lookup time for the retreive component (GET) and has component functions faster
using EntityComponentsCollection = std::unordered_map<EntityID, std::unordered_map<ComponentType, ComponentID>>;
//This is not neccessary but it needed to increase lookup times from component ID to entity ID so 
//you can easily get the parent entityt of a component and find the other components for that entity
//Although this could be done differently with only components, we need to get entity to allow it to go through it
using ComponentEntitiesCollection = std::unordered_map<ComponentID, EntityID>;
class EntityMapper
{
private:
	std::array<ComponentCollection, MAX_COMPONENT_TYPES> m_allComponents;
	std::array<ComponentIndex, MAX_COMPONENT_TYPES> m_nextAvailableComponentIndices;

	//TODO: perhaps this would be more efficient if we made this an array and just 
	// held an index at the places
	//that correspond to a component
	EntityComponentsCollection m_entityComponentIds;
	ComponentEntitiesCollection m_componentEntityIds;

	EntityID m_nextAvailableEntityID;
public:
	//If true, will place the components of the same entity in the same column
	//in order to maximize same entity- component searching without knowing entity ID
	//but comes at the cost of a little extra O(1) searching
	static constexpr bool DO_COMPONENT_LINEAR_PLACEMENT = false;

private:
	ComponentNumericType GetRowIndexForComponentID(const ComponentID& compId) const;
	ComponentIndex GetColIndexForComponentID(const ComponentID& compId) const;
	ComponentType GetComponentTypeFromID(const ComponentID& compId) const;
	
	ComponentRowIndex GetRowIndexForComponentType(const ComponentType& type) const;
	ComponentID CreateComponentID(const ComponentType& type, const ComponentIndex& colIndex) const;

	//TODO: if optional is too slow for performance optimize it to instead have a sentinel value
	//like the max value of the type

	///// <summary>
	///// Will find the index of the row that is reserved for the component type 
	///// (imagine in 2D array, this is the row index)
	///// </summary>
	///// <typeparam name="T"></typeparam>
	///// <param name="compType"></param>
	///// <returns></returns>
	//template<typename T>
	//ComponentRowIndex GetComponentRowIndex()
	//{
	//	if (compType == ComponentType::None)
	//	{
	//		compType = GetComponentFromType<T>();
	//	}

	//	if ((compType & m_registedTypes) == 0) return INVALID_COMPONENT_ROW_INDEX;
	//	else
	//	{
	//		//TODO: is naive search better than a .find on heap allocated
	//		//unordered map with indices?
	//		for (int i = 0; i < m_componentTypeIndices; i++)
	//		{
	//			if (m_componentTypeIndices[i] == ComponentType::None) return INVALID_COMPONENT_ROW_INDEX;
	//			if (m_componentTypeIndices[i] == compType) return i;
	//		}
	//		return INVALID_COMPONENT_ROW_INDEX;

	//		/*std::string compTypeStr = ToString(compType);
	//		auto iterator= std::find_if(m_allComponents.begin(), m_allComponents.end(),
	//			[](const ComponentCollectionType& collection) -> bool
	//			{
	//				if (collection.empty()) continue;
	//				return collection[0].type().name() == compTypeStr);
	//			});

	//		if (iterator == m_allComponents.end()) return -1;
	//		else return iterator - m_allComponents.begin();*/
	//	}
	//}

	EntityComponentsCollection::iterator GetEntityIteratorMutable(const EntityID& id);
	EntityComponentsCollection::const_iterator GetEntityIterator(const EntityID& id) const;

	void FindNextAvailableEntityID();
	void StoreComponentForEntity(const EntityID& entityId, const ComponentID& compId);

	void FindNextAvailableComponentIndex(const ComponentRowIndex& rowIndex);

	template<typename T>
	T* TryConvertToComponentType(std::any& anyType, const ComponentType& compType)
	{
		T* ptr = nullptr;
		try
		{
			ptr = &(std::any_cast<T&>(anyType));
		}
		catch (const std::bad_any_cast& e)
		{
			Utils::Log(this, Utils::LogType::Error, std::format("Tried to get component of type: {} "
				"but casting to type: {} failed since actual type is: {}",
				ToString(compType), typeid(T).name(), anyType.type().name()));
			return nullptr;
		}

		return ptr;
	}

public:
	EntityMapper();

	bool HasEntityWithID(const EntityID& entityID) const;
	EntityID ReserveAvailableEntityID();

	/// <summary>
	/// Will add the specified type of component. To improve performance and prevent the need for
	/// pointers/indirection and performance lost, the component data is moved.
	/// Note: only rvalues are allowed since it helps performance since a component can not be added as a 
	/// reference to the structure that stores componnents
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="component"></param>
	/// <returns></returns>
	template<typename T>
	requires std::is_rvalue_reference_v<T&&>
	std::optional<ComponentID> TryAddComponent(const EntityID& entityId, T&& component, 
		const ComponentType& compType = ComponentType::None, T** outComponent=nullptr)
	{
		if (!Utils::Assert(this, std::is_copy_assignable_v<T> , std::format("Tried to add component of type: {} "
			"to entity id: {} but that type does not have a valid move assignment operator to move the componnent",
			typeid(T).name(), std::to_string(entityId)))) return std::nullopt;

		if (!Utils::Assert(this, HasEntityWithID(entityId), std::format("ENTITY:MAPPER: Tried to add component of type: {} "
			"to entity id: {} but that entity ID is not reserved! Reserved: {}", 
			ToString(compType), std::to_string(entityId), Utils::ToStringIterable(m_entityComponentIds)))) return std::nullopt;

		ComponentType validCompType = compType;
		if (validCompType == ComponentType::None)
		{
			validCompType = GetComponentFromType<T>();
		}

		const ComponentRowIndex componentRowIndex = GetRowIndexForComponentType(validCompType);
		ComponentID compId = -1;
		Utils::Log(std::format("Component index: {}/{}", std::to_string(componentRowIndex), std::to_string(m_allComponents.size())));
		
		T* outCompPtr = nullptr;
		if (DO_COMPONENT_LINEAR_PLACEMENT)
		{
			auto existingEntityIt = m_entityComponentIds.find(entityId);
			Utils::Log(std::format("Attempting to add component: {}. exists: {} is empty: {}",
			ToString(compType), std::to_string(existingEntityIt != m_entityComponentIds.end()), std::to_string(existingEntityIt->second.empty())));

			if (existingEntityIt != m_entityComponentIds.end() && !(existingEntityIt->second.empty()))
			{
				const ComponentIndex existingColIndex= GetColIndexForComponentID(existingEntityIt->second.begin()->second);

				//Note: this should use the copy assignment operator to prevent copies since those would only
				//increase data storage without providing any benefit (since changes would be made to copy here)
				m_allComponents[componentRowIndex][existingColIndex] = component;
				outCompPtr = TryConvertToComponentType<T>(m_allComponents[componentRowIndex][existingColIndex], validCompType);
				outComponent = &(outCompPtr);

				compId = CreateComponentID(validCompType, existingColIndex);
				StoreComponentForEntity(entityId, compId);

				if (m_nextAvailableComponentIndices[componentRowIndex] == existingColIndex)
				{
					FindNextAvailableComponentIndex(componentRowIndex);
				}
				return compId;
			}
		}
		
		//If this is the first one, we need to resize space so we can index any space (since it reserves and fills
		//with empty) with linear placement needs indicies not in order, whereas if we do not do it we can just reserve
		if (m_allComponents[componentRowIndex].empty())
		{
			if (DO_COMPONENT_LINEAR_PLACEMENT) m_allComponents[componentRowIndex].resize(MAX_ENTITIES);
			else m_allComponents[componentRowIndex].reserve(MAX_ENTITIES);
		}
		
		const ComponentIndex elementIndex = m_nextAvailableComponentIndices[componentRowIndex];
		Utils::Log(std::format("Element index is; {}", std::to_string(elementIndex)));
		if (!Utils::Assert(this, elementIndex <= MAX_COMPONENT_COL_INDEX, std::format("ENTITY MAPPER: Tried to "
			"add component type: {} but there is already a max entity limit: {}",
			typeid(T).name(), std::to_string(MAX_COMPONENT_COL_INDEX))))
		{
			return std::nullopt;
		}

		Utils::Log(std::format("Assign componentn to mapper. found r: {}/{} c: {}/{}", 
			std::to_string(componentRowIndex), std::to_string(m_allComponents.size()-1), 
			std::to_string(elementIndex), std::to_string(m_allComponents[componentRowIndex].size()-1)));

		if (elementIndex == 0) m_allComponents[componentRowIndex].emplace_back(component);
		else m_allComponents[componentRowIndex][elementIndex] = component;

		outCompPtr = TryConvertToComponentType<T>(m_allComponents[componentRowIndex][elementIndex], validCompType);
		outComponent = &(outCompPtr);

		compId = CreateComponentID(validCompType, elementIndex);
		StoreComponentForEntity(entityId, compId);

		Utils::Log("finding nextg comp index");
		FindNextAvailableComponentIndex(componentRowIndex);
		return compId;
	}

	//TODO: implement removing components which should also place the corresponding available index counter to it

	template<typename T>
	T* TryGetComponent(const EntityID& entityId, const ComponentType& compType)
	{
		if (!Utils::Assert(this, HasEntityWithID(entityId), std::format("ENTITY MAPPER: Tried to get component of type: {} "
			"to entity id : {} but that entity ID is not reserved!", 
			ToString(compType), std::to_string(entityId)))) return -1;

		auto entityIt = m_entityComponentIds.find(entityId);
		if (entityIt == m_entityComponentIds.end()) return nullptr;

		auto compIt = entityIt->second.find(compType);
		if (compIt == entityIt->second.end()) return nullptr;
		
		const ComponentID compId = compIt->second;

		std::any& anyType = m_allComponents[GetRowIndexForComponentID(compId)][GetColIndexForComponentID(compId)];
		return TryConvertToComponentType<T>(anyType, compType);
	}

	template<typename T>
	T* TryGetComponent(const ComponentID& compId)
	{
		ComponentRowIndex rowIndex= GetRowIndexForComponentID(compId);
		if (m_allComponents.size() >= rowIndex) return nullptr;

		ComponentIndex colIndex= GetColIndexForComponentID(compId);
		if (m_allComponents[rowIndex].size() >= colIndex) return nullptr;

		return TryConvertToComponentType<T>(m_allComponents[rowIndex][colIndex], GetComponentTypeFromID(compId));
	}

	std::optional<EntityID> TryGetEntityWithComponent(const ComponentID& compId);

	/// <summary>
	/// Returns all components of the type. Note: this is a little costly due to the
	/// any cast conversion required for each component
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="type"></param>
	/// <returns></returns>
	template<typename T>
	std::vector<T*> TryGetComponentsOfType(const ComponentType& type, const std::function<void(T*, const EntityID&)>& action=nullptr)
	{
		const ComponentRowIndex rowIndex = GetRowIndexForComponentType(type);
		std::vector<T*> result = {};

		int col = -1;
		for (auto& maybeComponent : m_allComponents[rowIndex])
		{
			col++;

			//NOTE: this is a lot more constly when linear placement since we don't know if empty cells
			//means end or continue, but we do know it means end when not doing it
			if (!maybeComponent.has_value())
			{
				if (DO_COMPONENT_LINEAR_PLACEMENT) continue;
				else break;
			}

			T* compPtr = TryConvertToComponentType<T>(maybeComponent, type);
			if (action != nullptr)
			{
				const ComponentID compId = CreateComponentID(type, col);
				std::optional<EntityID> maybeId = TryGetEntityWithComponent(compId);

				if (!Utils::Assert(this, maybeId.has_value(), std::format("ENTITY MAPPER: Tried to get all components of type: {} "
					"but failed to retrieve its entity id from col index: {}", ToString(type), std::to_string(col)))) return {};
				action(compPtr, maybeId.value());
			}
			result.push_back(compPtr);
		}
		return result;
	}


};
