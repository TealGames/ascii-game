#include "pch.hpp"
#include "Entity.hpp"
//#include "EntityMapper.hpp"
#include "HelperFunctions.hpp"

namespace ECS
{
	//TODO: add a reserved entity name ssytem and not allowing those names to be used

	/// <summary>
	/// This will create the entity with name and mapper.
	/// Note: the transform will be copied and then moved into the component storage 
	/// </summary>
	/// <param name="name"></param>
	/// <param name="mapper"></param>
	/// <param name="transform"></param>
	Entity::Entity(const std::string& name, entt::registry& mapper, const TransformData& transform) :
		m_name(name), m_Name(m_name), m_entityMapper(mapper),
		//m_Id(m_entityMapper.ReserveAvailableEntityID()),
		m_Id(m_entityMapper.create()),
		m_Transform(AddComponent<TransformData>(transform)) //m_componentIDs{}
	{
		
	}

	//Entity::Entity(const std::string& name, entt::registry& mapper, TransformData&& transform) :
	//	m_name(name), m_Name(m_name), m_entityMapper(mapper),
	//	//m_Id(m_entityMapper.ReserveAvailableEntityID()),
	//	m_Id(m_entityMapper.create()),
	//	m_Transform(std::move(transform)), m_componentIDs{}
	//{

	//}

	/// <summary>
	/// This will get the transform ref of the stored transform 
	/// after making a copy of the argument first
	/// </summary>
	/// <param name="transform"></param>
	/// <returns></returns>
	/*TransformData& Entity::GetTransformRef(const TransformData& transform)
	{
		TransformData transformCopy = transform;
		
		TransformData* singleOutPtr = nullptr;
		TransformData** doubleOutPtr = &singleOutPtr;
		TryAddComponent<TransformData>(std::move(transformCopy), doubleOutPtr);

		Utils::Assert(this, Utils::IsValidPointer(doubleOutPtr), std::format("Tried to bind transform data for entity: {} "
			"using copy but failed to retrieve it after it has been added", m_Name));

		return *(*doubleOutPtr);
	}

	/// <summary>
	/// This will get the transform by directly moving the transform rvalue
	/// </summary>
	/// <param name="transform"></param>
	/// <returns></returns>
	TransformData& Entity::GetTransformRef(TransformData&& transform)
	{
		TransformData* singleOutPtr = nullptr;
		TransformData** doubleOutPtr = &singleOutPtr;
		TryAddComponent<TransformData>(std::move(transform), doubleOutPtr);

		Utils::Assert(this, Utils::IsValidPointer(doubleOutPtr), std::format("Tried to bind transform data for entity: {} "
			"using move but failed to retrieve it after it has been added", m_Name));

		return *(*doubleOutPtr);
	}*/

	/*ComponentCollectionType::iterator Entity::GetComponentIDIteratorMutable(const ComponentType& type)
	{
		return m_componentIDs.find(type);
	}

	ComponentCollectionType::const_iterator Entity::GetComponentIDIterator(const ComponentType& type) const
	{
		return m_componentIDs.find(type);
	}*/

	/*bool Entity::HasComponent(const ComponentType& type) const
	{
		return GetComponentIDIterator(type) != m_componentIDs.end();
	}*/

	std::string Entity::ToString() const
	{
		return "";
		/*auto components= Utils::GetKeysFromMap<ComponentType, ComponentID>(m_componentIDs.begin(), m_componentIDs.end());
		std::string componentsStr = ::ToString(MergeComponents(components));*/
		//return std::format("['{}' c:{}]", m_Name, componentsStr);
	}

	//int Entity::GetComponentTypeCount(const ComponentType& type) const
	//{
	//	//TODO: optimization could be to batch group components of the same type
	//	//if it is allowed in order to have O(1) lookup time
	//	int count = 0;
	//	for (const auto& component : m_components)
	//	{
	//		if (component.second->m_Type == type)count++;
	//	}
	//	return count;
	//}

	//bool Entity::HasComponentOfType(const ComponentType& type) const
	//{
	//	return GetComponentTypeCount(type) > 0;
	//}

	//ComponentMapType::iterator Entity::GetComponentIteratorWithIdMutable(const int& componentId)
	//{
	//	return std::find(m_components.begin(), m_components.end(), componentId);
	//}

	//ComponentMapType::const_iterator Entity::GetComponentIteratorWithId(const int& componentId) const
	//{
	//	return std::find(m_components.begin(), m_components.end(), componentId);
	//}

	//bool Entity::HasComponentWithId(const int& componentId) const
	//{
	//	return GetComponentIteratorWithId(componentId) != m_components.end();
	//}

	//const Component* Entity::TryGetComponentWithId(const int& componentId) const
	//{
	//	ComponentMapType::const_iterator componentIt = GetComponentIteratorWithId(componentId);
	//	if (componentIt == m_components.end()) return nullptr;
	//	return componentIt->second;
	//}

	//bool Entity::TryAddComponent(Component* component, const int& componentId = -1)
	//{
	//	if (GetComponentTypeCount(component->m_Type) >= component->GetMaxPerObject())
	//	{
	//		std::string error = std::format("Tried to place a component of type: {} "
	//			"on gameObject {} but it already contains the max: {}",
	//			ToString(component->m_Type), m_Name, std::to_string(component->GetMaxPerObject()));
	//		Utils::Log(Utils::LogType::Error, error);
	//		return false;
	//	}
	//	static int lastId = -1;

	//	if (componentId != -1)
	//	{
	//		if (componentId < -1)
	//		{
	//			std::string error = std::format("Tried to add a component of {} to gameObject with name: {} "
	//				"but the component id ({}) is not allowed!", ToString(component->m_Type), m_Name, std::to_string(componentId));
	//			Utils::Log(Utils::LogType::Error, error);
	//			return false;
	//		}

	//		if (HasComponentWithId(componentId))
	//		{
	//			std::string error = std::format("Tried to add a component of {} to gameObject with name: {} "
	//				"but the component id ({}) is already taken!", ToString(component->m_Type), m_Name, std::to_string(componentId));
	//			Utils::Log(Utils::LogType::Error, error);
	//			return false;
	//		}
	//		m_components.emplace(componentId, component);
	//		lastId = componentId;
	//	}
	//	else
	//	{
	//		m_id = ++lastId;
	//		m_components.emplace(lastId, component);
	//	}

	//	return true;
	//}

	//bool Entity::TryRemoveComponent(const int& componentId)
	//{
	//	ComponentMapType::const_iterator componentIt = GetComponentIteratorWithId(componentId);
	//	if (componentIt == m_components.end()) return false;
	//	m_components.erase(componentIt);
	//}
}



