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
#include "IValidateable.hpp"

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
	class Entity : public IValidateable
	{
	private:
		//EntityMapper& m_entityMapper;
		entt::registry& m_entityMapper;
		//ComponentCollectionType m_componentIDs;

		std::string m_name;
		//TODO: perhaps we should also make a custom type that stores the actual type contained
		//or maybe we can contain that data based on what index it is at?
		std::vector<ComponentData*> m_components;

		std::string m_sceneName;
	public:
		//TODO: maybe replace id with GUID (UIUD is available in boost library)k
		//const EntityID m_Id;
		const EntityID m_Id;
		TransformData& m_Transform;

		//TODO: is this really the best location for something like this?
		static const char* GLOBAL_SCENE_NAME;

	private:

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="name"></param>
		/// <param name="objectId"></param>
		Entity(const std::string& name, entt::registry& mapper, const TransformData& transform);

		std::string GetName() const;

		/// <summary>
		/// Will set the scene name as the name specified. Note: since it has no reference to a scene,
		/// this name is not checked so make sure it is written correctly
		/// </summary>
		/// <param name="sceneName"></param>
		void SetScene(const std::string& sceneName);
		std::string GetSceneName() const;
		bool IsGlobal() const;

		template<typename T, typename... Args>
		requires IsComponent<T>
		T& AddComponent(Args&&... args)
		{
			if (!Assert(this, !HasComponent<T>(), std::format("Tried to add component of type: {} to {} "
				"but it already has this type (and duplicates are not allowed)", typeid(T).name(),
				ToString()))) throw std::invalid_argument("Attempted to add duplicate component");

			T& result= m_entityMapper.emplace<T>(m_Id, std::forward<Args>(args)...);
			ComponentData* componentData = &result;
			componentData->m_Entity = this;
			componentData->InitFields();

			m_components.push_back(componentData);
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
			ComponentData* componentData = &result;
			componentData->m_Entity = this;
			componentData->InitFields();

			m_components.push_back(componentData);
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

			/*
			T& result = m_entityMapper.emplace<T>(m_Id);
			ComponentData& componentData = static_cast<ComponentData&>(result);
			componentData.m_Entity = this;
			m_components.push_back(&componentData);
			*/

			T& result = m_entityMapper.emplace_or_replace<T>(m_Id, component);
			ComponentData* componentData = &result;
			componentData->m_Entity = this;
			componentData->InitFields();

			m_components.push_back(componentData);
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

		const ComponentData* TryGetComponentAtIndex(const size_t& index) const;
		ComponentData* TryGetComponentAtIndexMutable(const size_t& index);

		const ComponentData* TryGetComponentWithName(const std::string& name) const;
		ComponentData* TryGetComponentWithNameMutable(const std::string& name);

		size_t TryGetIndexOfComponent(const ComponentData* component) const;
		size_t TryGetIndexOfComponent(const std::string& componentName) const;

		std::string TryGetComponentName(const ComponentData* component) const;

		/// <summary>
		/// Will return all components as base type for this entity. 
		/// Note: even thoguh it is const, since each element is a pointer, the component
		/// is modifiable, but no other components can be added/removed/changed
		/// </summary>
		/// <returns></returns>
		const std::vector<ComponentData*>& GetAllComponentsMutable() const;

		bool Validate() override;

		std::string ToString() const;
		static std::string ToString(const EntityID& id);

		bool operator==(const ECS::Entity& other) const;
	};
}

