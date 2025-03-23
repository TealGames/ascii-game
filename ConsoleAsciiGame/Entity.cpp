#include "pch.hpp"
#include "Entity.hpp"
//#include "EntityMapper.hpp"
#include "HelperFunctions.hpp"

namespace ECS
{
	const char* ECS::Entity::GLOBAL_SCENE_NAME = "Global";
	//TODO: add a reserved entity name ssytem and not allowing those names to be used

	/// <summary>
	/// This will create the entity with name and mapper.
	/// Note: the transform will be copied and then moved into the component storage 
	/// </summary>
	/// <param name="name"></param>
	/// <param name="mapper"></param>
	/// <param name="transform"></param>
	Entity::Entity(const std::string& name, entt::registry& mapper, const TransformData& transform) :
		m_name(name), m_entityMapper(mapper),
		//m_Id(m_entityMapper.ReserveAvailableEntityID()),
		m_Id(m_entityMapper.create()),
		m_Transform(AddComponent<TransformData>(transform)), 
		m_components() //m_componentIDs{}
	{
	}

	std::string Entity::GetName() const
	{
		return m_name;
	}

	void Entity::SetScene(const std::string& sceneName)
	{
		m_sceneName = sceneName;
	}
	std::string Entity::GetSceneName() const
	{
		return m_sceneName;
	}
	bool Entity::IsGlobal() const
	{
		return m_sceneName == GLOBAL_SCENE_NAME;
	}

	const ComponentData* Entity::TryGetComponentAtIndex(const size_t& index) const
	{
		if (index < 0 || index >= m_components.size()) 
			return nullptr;

		return m_components[index];
	}
	ComponentData* Entity::TryGetComponentAtIndexMutable(const size_t& index)
	{
		if (index < 0 || index >= m_components.size()) 
			return nullptr;

		return m_components[index];
	}

	const ComponentData* Entity::TryGetComponentWithName(const std::string& name) const
	{
		for (const auto& component : m_components)
		{
			if (component == nullptr) continue;
			if (Utils::FormatTypeName(typeid(*component).name()) == name)
				return component;
		}
		return nullptr;
	}
	ComponentData* Entity::TryGetComponentWithNameMutable(const std::string& name)
	{
		for (auto& component : m_components)
		{
			if (component == nullptr) continue;
			if (Utils::FormatTypeName(typeid(*component).name()) == name)
				return component;
		}
		return nullptr;
	}

	size_t Entity::TryGetIndexOfComponent(const ComponentData* targetComponent) const
	{
		size_t index = 0;
		for (const auto& component : m_components)
		{
			if (component == nullptr) continue;
			if (component == targetComponent) 
				return index;

			index++;
		}
		return -1;
	}
	size_t Entity::TryGetIndexOfComponent(const std::string& componentName) const
	{
		size_t index = 0;
		for (const auto& component : m_components)
		{
			if (component == nullptr) continue;
			if (Utils::FormatTypeName(typeid(*component).name()) == componentName)
				return index;

			index++;
		}
		return -1;
	}

	std::string Entity::TryGetComponentName(const ComponentData* component) const
	{
		return Utils::FormatTypeName(typeid(*component).name());
	}

	std::string Entity::ToString() const
	{
		std::string componentNames = "Components: ";
		if (!m_components.empty())
		{
			for (size_t i=0; i<m_components.size(); i++)
			{
				if (i != 0) componentNames += ", ";
				componentNames += Utils::FormatTypeName(typeid(*m_components[i]).name());
			}
			//componentNames = Utils::ToStringIterable<std::vector<std::string>, std::string>(m_components);
		}

		return std::format("['{}'(ID: {})-> {}]", GetName(), Entity::ToString(m_Id), componentNames);
		/*auto components= Utils::GetKeysFromMap<ComponentType, ComponentID>(m_componentIDs.begin(), m_componentIDs.end());
		std::string componentsStr = ::ToString(MergeComponents(components));*/
		//return std::format("['{}' c:{}]", m_Name, componentsStr);
	}

	bool Entity::Validate()
	{
		bool passesValidation = true;
		for (const auto& component : m_components)
		{
			if (!component->Validate()) passesValidation = false;
		}
		return passesValidation;
	}

	std::string Entity::ToString(const EntityID& id)
	{
		return std::to_string(static_cast<uint32_t>(id));
	}

	bool Entity::operator==(const ECS::Entity& other) const
	{
		//Note: not fully guaranteed to be equal, but highlight likely
		return m_name == other.m_name && m_Id == other.m_Id 
			&& m_components == other.m_components;
	}

	const std::vector<ComponentData*>& Entity::GetAllComponentsMutable() const
	{
		return m_components;
	}
}



