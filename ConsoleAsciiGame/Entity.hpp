#pragma once
#include <map>
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <entt/entt.hpp>
#include <tuple>
#include "HelperFunctions.hpp"
#include "TransformData.hpp"
#include "ComponentType.hpp"
#include "Debug.hpp"

namespace ECS
{
	template<typename T>
	concept IsComponent = std::is_base_of_v<ComponentData, T>;

	template<typename T>
	requires IsComponent<T>
	struct EntityComponentPair
	{
		//Note: the types are pointers so this object
		//can easily be copied into things like optional, vector, etc

		ECS::Entity* m_Entity;
		T* m_Data;

		EntityComponentPair(ECS::Entity& entity, T& data) :
			m_Entity(&entity), m_Data(&data) {}
	};

	template<typename ...Args>
	requires Utils::HasAtLeastOneArg<Args...> 
			 && Utils::AllSameBaseType<ComponentData, Args...>
	struct EntityComponents
	{
		ECS::Entity* m_Entity;

		using TupleType = std::tuple<typename Utils::ToPointerType<Args>::Type...>;
		TupleType m_Data;

		EntityComponents(ECS::Entity& entity, Args&... data) 
			: m_Entity(&entity), m_Data(std::make_tuple(&data...)) 
		{
		}

		template<std::size_t index>
		auto GetAt() -> typename std::remove_pointer_t<std::tuple_element_t<index, TupleType>>&
		{
			return *(std::get<index>(m_Data));
		}
	};

	//using ComponentCollectionType = std::unordered_map<ComponentType, ComponentID>;
	using EntityID = entt::entity;
	class Entity
	{
	private:
		//EntityMapper& m_entityMapper;
		entt::registry& m_entityMapper;
		//ComponentCollectionType m_componentIDs;

		std::string m_name;
		//TODO: perhaps we should also make a custom type that stores the actual type contained
		//or maybe we can contain that data based on what index it is at?
		std::vector<ComponentData*> m_components;

	public:
		const std::string& m_Name;
		//TODO: maybe replace id with GUID (UIUD is available in boost library)
		//const EntityID m_Id;
		const EntityID m_Id;
		TransformData& m_Transform;

	private:
		/*TransformData& GetTransformRef(const TransformData& data);
		TransformData& GetTransformRef(TransformData&& data);*/

		/*ComponentCollectionType::iterator GetComponentIDIteratorMutable(const ComponentType& type);
		ComponentCollectionType::const_iterator GetComponentIDIterator(const ComponentType& type) const;*/

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="name"></param>
		/// <param name="objectId"></param>
		Entity(const std::string& name, entt::registry& mapper, const TransformData& transform);
		//Entity(const std::string& name, entt::registry& mapper, TransformData&& transform);

		//We do not allow copying since the component data is stored in the entity mapper
		//and copying would delete obj and not data since deleting/removing data is not implemented
		//in the entity mapper yet
		//Entity(Entity&& entity) = delete;

		//bool HasComponent(const ComponentType& type) const;

		template<typename T, typename... Args>
		requires IsComponent<T>
		T& AddComponent(Args&&... args)
		{
			if (!Assert(this, !HasComponent<T>(), std::format("Tried to add component of type: {} to {} "
				"but it already has this type (and duplicates are not allowed)", typeid(T).name(),
				ToString()))) throw std::invalid_argument("Attempted to add duplicate component");

			T& result= m_entityMapper.emplace<T>(m_Id, std::forward<Args>(args)...);
			ComponentData& componentData = static_cast<ComponentData&>(result);
			componentData.m_Entity = this;
			m_components.push_back(&componentData);
			return result;
		}

		/// <summary>
		/// Will add the component of type to this entity using no argument/default constructor
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		template<typename T>
		requires IsComponent<T> && std::is_default_constructible_v<T>
		T& AddComponent()
		{
			if (!Assert(this, !HasComponent<T>(), std::format("Tried to add component of type: {} to {} "
				"but it already has this type (and duplicates are not allowed)", typeid(T).name(), 
				ToString()))) throw std::invalid_argument("Attempted to add duplicate component");

			T& result= m_entityMapper.emplace<T>(m_Id);
			ComponentData& componentData = static_cast<ComponentData&>(result);
			componentData.m_Entity = this;
			m_components.push_back(&componentData);
			return result;
		}

		/// <summary>
		/// Will add the component with data to this entity
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="component"></param>
		/// <returns></returns>
		template<typename T>
		requires IsComponent<T>
		T& AddComponent(const T& component)
		{
			if (!Assert(this, !HasComponent<T>(), std::format("Tried to add component of type: {} to {} "
				"but it already has this type (and duplicates are not allowed)", typeid(T).name(),
				ToString()))) throw std::invalid_argument("Attempted to add duplicate component");

			T& result = m_entityMapper.emplace_or_replace<T>(m_Id, component);
			ComponentData& componentData = static_cast<ComponentData&>(result);
			componentData.m_Entity = this;
			m_components.push_back(&componentData);
			return result;
		}

		template<typename T>
		requires IsComponent<T>
		bool HasComponent() const
		{
			//TODO: maybe try checking if checking vector of all this entity's component
			//names is faster than doing try get
			return m_entityMapper.try_get<T>(m_Id) != nullptr;
		}

		template<typename T>
		requires IsComponent<T>
		T* TryGetComponentMutable()
		{
			return m_entityMapper.try_get<T>(m_Id);
		}

		template<typename T>
		requires IsComponent<T>
		const T* TryGetComponent()
		{
			return m_entityMapper.try_get<T>(m_Id);
		}

		/// <summary>
		/// Will return all components as base type for this entity. 
		/// Note: even thoguh it is const, since each element is a pointer, the component
		/// is modifiable, but no other components can be added/removed/changed
		/// </summary>
		/// <returns></returns>
		const std::vector<ComponentData*>& GetAllComponentsMutable() const;

		std::string ToString() const;
		static std::string ToString(const EntityID& id);

		bool operator==(const ECS::Entity& other) const;

#ifdef ENABLE_JSON_LIB
		friend void from_json(const Json& json, Entity& entity);
#endif
	};
}

