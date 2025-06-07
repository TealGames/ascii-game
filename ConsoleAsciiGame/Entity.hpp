#pragma once
#include <map>
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <tuple>
#include "HelperFunctions.hpp"
#include "ComponentType.hpp"
#include "Debug.hpp"
#include "IValidateable.hpp"
#include "EntityID.hpp"

class TransformData;
class EntityData;

namespace ECS
{
	class EntityRegistry;

	template<typename T>
	requires std::is_base_of_v<Component, T>
	struct EntityComponentPair
	{
		//Note: the types are pointers so this object
		//can easily be copied into things like optional, vector, etc

		EntityData* m_Entity;
		T* m_Data;

		EntityComponentPair(EntityData& entity, T& data) :
			m_Entity(&entity), m_Data(&data) {}
	};

	template<typename ...Args>
	requires Utils::HasAtLeastOneArg<Args...> 
			 && Utils::AllSameBaseType<Component, Args...>
	struct EntityComponents
	{
		EntityData* m_Entity;

		using TupleType = std::tuple<typename Utils::ToPointerType<Args>::Type...>;
		TupleType m_Data;

		EntityComponents(EntityData& entity, Args&... data)
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

	class Entity : public IValidateable
	{
	private:
		//EntityMapper& m_entityMapper;
		EntityRegistry* m_registry;
		//ComponentCollectionType m_componentIDs;

		//TODO: perhaps we should also make a custom type that stores the actual type contained
		//or maybe we can contain that data based on what index it is at?

	public:
		//TODO: maybe replace id with GUID (UIUD is available in boost library)k
		//const EntityID m_Id;
		const EntityID m_Id;

		//TODO: could we maybe comine all entity falgs into one bool/ enum

		


	private:
		EntityData& GetEntityDataMutable();
		const EntityData& GetEntityData() const;

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="name"></param>
		/// <param name="objectId"></param>
		Entity(EntityRegistry& mapper, const EntityID id);

		//EntityRegistry& GetRegistryMutable();
		//const EntityRegistry& GetRegistry() const;

		std::string GetName() const;
		TransformData& GetTransformMutable();

		/// <summary>
		/// Will set the scene name as the name specified. Note: since it has no reference to a scene,
		/// this name is not checked so make sure it is written correctly
		/// </summary>
		/// <param name="sceneName"></param>
		void SetScene(const std::string& sceneName);
		std::string GetSceneName() const;
		bool IsGlobal() const;

		template<typename T, typename... Args>
		requires std::is_base_of_v<Component, T>
		T& AddComponent(Args&&... args)
		{
			if (!Assert(this, !HasComponent<T>(), std::format("Tried to add component of type: {} to {} "
				"but it already has this type (and duplicates are not allowed)", typeid(T).name(),
				ToString()))) throw std::invalid_argument("Attempted to add duplicate component");

			T& result= m_registry.emplace<T>(m_Id, std::forward<Args>(args)...);
			Component* componentData = &result;
			componentData->m_entity = this;
			componentData->InitFields();

			m_components.emplace_back(componentData);
			return result;
		}

		/// <summary>
		/// Will add the component of type to this entity using no argument/default constructor
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		template<typename T>
		requires std::is_base_of_v<Component, T> && std::is_default_constructible_v<T>
		T& AddComponent()
		{
			if (!Assert(this, !HasComponent<T>(), std::format("Tried to add component of type: {} to {} "
				"but it already has this type (and duplicates are not allowed)", typeid(T).name(), 
				ToString()))) throw std::invalid_argument("Attempted to add duplicate component");

			T& result= m_registry.emplace<T>(m_Id);
			Component* componentData = &result;
			componentData->m_entity = this;
			componentData->InitFields();

			m_components.emplace_back(componentData);
			return result;
		}

		/// <summary>
		/// Will add the component with data to this entity
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="component"></param>
		/// <returns></returns>
		template<typename T>
		requires std::is_base_of_v<Component, T>
		T& AddComponent(const T& component)
		{
			if (!Assert(this, !HasComponent<T>(), std::format("Tried to add component of type: {} to {} "
				"but it already has this type (and duplicates are not allowed)", typeid(T).name(),
				ToString()))) throw std::invalid_argument("Attempted to add duplicate component");

			T& result = m_registry.emplace_or_replace<T>(m_Id, component);
			Component* componentData = &result;
			componentData->m_entity = this;
			componentData->InitFields();

			m_components.emplace_back(componentData);
			return result;
		}

		template<typename T>
		requires std::is_base_of_v<Component, T>
		bool HasComponent() const
		{
			//TODO: maybe try checking if checking vector of all this entity's component
			//names is faster than doing try get
			return m_registry.try_get<T>(m_Id) != nullptr;
		}

		/// <summary>
		/// Will return true if the target component is stored in this entity.
		/// Note: this is slower than checking if it contains a type because it requires
		/// O(n) complexity to check all components
		/// </summary>
		/// <param name="targetComponent"></param>
		/// <returns></returns>
		/*bool HasComponent(const ComponentData* targetComponent) const
		{
			if (targetComponent == nullptr) 
				return false;

			if (targetComponent->GetEntitySafe() != *this)
				return false;

			for (const auto& component : m_components)
			{
				if (component == targetComponent)
					return true;
			}
			return false;
		}*/


		template<typename T>
		requires std::is_base_of_v<Component, T>
		T* TryGetComponentMutable()
		{
			return m_registry.try_get<T>(m_Id);
		}

		template<typename T>
		requires std::is_base_of_v<Component, T>
		const T* TryGetComponent() const
		{
			return m_registry.try_get<T>(m_Id);
		}

		/*const ComponentData* TryGetComponentAtIndex(const size_t& index) const;
		ComponentData* TryGetComponentAtIndexMutable(const size_t& index);

		const ComponentData* TryGetComponentWithName(const std::string& name) const;
		ComponentData* TryGetComponentWithNameMutable(const std::string& name);

		size_t TryGetIndexOfComponent(const ComponentData* component) const;
		size_t TryGetIndexOfComponent(const std::string& componentName) const;*/

		std::string TryGetComponentName(const Component* component) const;

		/// <summary>
		/// Will return all components as base type for this entity. 
		/// Note: even thoguh it is const, since each element is a pointer, the component
		/// is modifiable, but no other components can be added/removed/changed
		/// </summary>
		/// <returns></returns>
		const std::vector<Component*>& GetAllComponentsMutable() const;

		bool Validate() override;

		std::string ToString() const;
		static std::string ToString(const EntityID& id);

		bool operator==(const ECS::Entity& other) const;
	};
}

