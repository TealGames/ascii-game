#include <format>
#include <vector>
#include <map>
#include <algorithm>
#include "Entity.hpp"
#include "EntityID.hpp"
#include "HelperFunctions.hpp"
#include "TextBuffer.hpp"

namespace ECS
{
	Entity::Entity(const std::string& name) :
		m_name(name), m_Name(m_name), m_Id(), m_components{}
	{
		static std::vector<int> takenIds = {};
		static int lastId = -1;

		/*if (objectId != -1)
		{
			if (objectId < -1)
			{
				std::string error = std::format("Tried to create a gameObject with name: {} "
					"and an id ({}) that is not allowed!", m_Name, std::to_string(objectId));
				Utils::Log(Utils::LogType::Error, error);
				return;
			}

			if (std::find(takenIds.begin(), takenIds.end(), objectId) != takenIds.end())
			{
				std::string error = std::format("Tried to create a gameObject with name: {} "
					"and an id ({}) that is no longer available!", m_Name, std::to_string(objectId));
				Utils::Log(Utils::LogType::Error, error);
				return;
			}
			m_id = objectId;
			takenIds.push_back(objectId);
		}
		else
		{
			m_id = ++lastId;
			takenIds.push_back(lastId);
		}*/
	}

	void Entity::Start()
	{
		if (m_components.empty()) return;
		for (const auto& component : m_components) component->Start();
	}

	void Entity::UpdateStart(float deltaTime)
	{
		if (m_components.empty()) return;
		for (const auto& component : m_components) component->UpdateStart(deltaTime);
	}

	void Entity::UpdateEnd(float deltaTime)
	{
		if (m_components.empty()) return;
		for (const auto& component : m_components) component->UpdateEnd(deltaTime);
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



