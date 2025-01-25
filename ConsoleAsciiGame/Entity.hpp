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
//#include "EntityMapper.hpp"
#include "HelperFunctions.hpp"
#include "TransformData.hpp"
#include "ComponentType.hpp"
#include <tuple>

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
		std::vector<std::string> m_componentNames;

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
			if (!Utils::Assert(this, !HasComponent<T>(), std::format("Tried to add component of type: {} to {} "
				"but it already has this type (and duplicates are not allowed)", typeid(T).name(),
				ToString()))) throw std::invalid_argument("Attempted to add duplicate component");

			T& result= m_entityMapper.emplace<T>(m_Id, std::forward<Args>(args)...);
			static_cast<ComponentData&>(result).m_Entity = this;
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
			if (!Utils::Assert(this, !HasComponent<T>(), std::format("Tried to add component of type: {} to {} "
				"but it already has this type (and duplicates are not allowed)", typeid(T).name(), 
				ToString()))) throw std::invalid_argument("Attempted to add duplicate component");

			T& result= m_entityMapper.emplace<T>(m_Id);
			static_cast<ComponentData&>(result).m_Entity = this;
			m_componentNames.push_back(typeid(T).name());
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
			if (!Utils::Assert(this, !HasComponent<T>(), std::format("Tried to add component of type: {} to {} "
				"but it already has this type (and duplicates are not allowed)", typeid(T).name(),
				ToString()))) throw std::invalid_argument("Attempted to add duplicate component");

			T& result = m_entityMapper.emplace_or_replace<T>(m_Id, component);
			static_cast<ComponentData&>(result).m_Entity = this;
			m_componentNames.push_back(typeid(T).name());
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
		T* TryGetComponent()
		{
			return m_entityMapper.try_get<T>(m_Id);
		}

		/// <summary>
		/// Will add the specified type of component. To improve performance and prevent the need for
		/// pointers/indirection and performance lost, the component data is moved.
		/// Note: only rvalues are allowed since it helps performance since a component can not be added as a 
		/// reference to the structure that stores componnents
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="component"></param>
		/// <param name="outComponent">Pointer to the component stored (since the actual componenent is moved) 
		/// since there is no point of keeping copies. Note: this should be a pointer to a null pointer (and not just null)
		/// if you want to retrieve the out value</param>
		/// <returns></returns>
		//template<typename T>
		//requires std::is_rvalue_reference_v<T&&>
		//bool TryAddComponent(T&& component, T** outComponent)
		//{
		//	if (!Utils::Assert(this, std::is_copy_assignable_v<T>, std::format("Tried to add component of type: {} "
		//		"to entity id: {} via ENTITY but that type does not have a valid move assignment operator to move the componnent",
		//		typeid(T).name(), m_Name))) return false;

		//	const ComponentType type = GetComponentFromType<T>();
		//	if (!Utils::Assert(this, !HasComponent(type), std::format("Tried to add the component type: {} to entity: {} "
		//		"but it already exists on this entity! Components: {}", 
		//		::ToString(type), m_Name, Utils::ToStringIterable(m_componentIDs)))) return false;

		//	Utils::Log(std::format("After adding comp: {} to entity: {} first: {} second: {}", 
		//		::ToString(type), m_Name, std::to_string(outComponent!=nullptr), std::to_string(outComponent? *outComponent!=nullptr : false)));

		//	//Note: we forward it to preserve the rvalue reference (and not need for the function to deduce again)
		//	std::optional<ComponentID> compID = m_entityMapper.TryAddComponent<T>(m_Id, std::forward<T>(component), type, outComponent);
		//	if (!Utils::Assert(this, compID.has_value(), std::format("Tried to add the component type: {} to entity: {} "
		//		"but the entity mapper failed to add it!", ::ToString(type), m_Name))) return false;

		//	m_componentIDs.emplace(type, compID.value());
		//	//outComponent = &(m_entityMapper.TryGetComponent<T>(m_Id, type));
		//	return true;
		//}

		//template<typename T>
		//requires std::is_rvalue_reference_v<T&&>
		//T* TryAddComponent(T&& component)
		//{
		//	T* singleOutPtr = nullptr;
		//	T** doubleOutPtr = &singleOutPtr;

		//	TryAddComponent<T>(std::move(component), doubleOutPtr);

		//	if (Utils::IsValidPointer(doubleOutPtr)) return *doubleOutPtr;
		//	return nullptr;
		//}

		//template<typename T>
		//T* TryGetComponent()
		//{
		//	const ComponentType type = GetComponentFromType<T>();
		//	auto it = GetComponentIDIteratorMutable(type);
		//	Utils::Log(std::format("Trying to get component: {} type: {} from entity: {}", typeid(T).name(), ::ToString(type), ToString()));

		//	return m_entityMapper.TryGetComponent<T>(it->second);
		//}

		std::string ToString() const;
		static std::string ToString(const EntityID& id);
	};
}

