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
		m_Transform(AddComponent<TransformData>(transform)), 
		m_components() //m_componentIDs{}
	{
	}

	std::string Entity::ToString() const
	{
		std::string componentNames = "[COMPONENT FIND NOT IMPLEMENTED]";
		if (!m_components.empty())
		{
			for (const auto& component : m_components) 
				componentNames += (typeid(component).name() + ',');
			//componentNames = Utils::ToStringIterable<std::vector<std::string>, std::string>(m_components);
		}

		return std::format("['{}'(ID: {})-> {}]", m_Name, Entity::ToString(m_Id), componentNames);
		/*auto components= Utils::GetKeysFromMap<ComponentType, ComponentID>(m_componentIDs.begin(), m_componentIDs.end());
		std::string componentsStr = ::ToString(MergeComponents(components));*/
		//return std::format("['{}' c:{}]", m_Name, componentsStr);
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



